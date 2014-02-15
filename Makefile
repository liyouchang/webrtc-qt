# We borrow heavily from the kernel build setup, though we are simpler since
# we don't have Kconfig tweaking settings on us.

# The implicit make rules have it looking for RCS files, among other things.
# We instead explicitly write all the rules we care about.
# It's even quicker (saves ~200ms) to pass -r on the command line.
MAKEFLAGS=-r

# The source directory tree.
srcdir := .
abs_srcdir := $(abspath $(srcdir))

# The name of the builddir.
builddir_name ?= out

# The V=1 flag on command line makes us verbosely print command lines.
ifdef V
  quiet=
else
  quiet=quiet_
endif

# Specify BUILDTYPE=Release on the command line for a release build.
BUILDTYPE ?= Debug

# Directory all our build output goes into.
# Note that this must be two directories beneath src/ for unit tests to pass,
# as they reach into the src/ directory for data with relative paths.
builddir ?= $(builddir_name)/$(BUILDTYPE)
abs_builddir := $(abspath $(builddir))
depsdir := $(builddir)/.deps

# Object output directory.
obj := $(builddir)/obj
abs_obj := $(abspath $(obj))

# We build up a list of every single one of the targets so we can slurp in the
# generated dependency rule Makefiles in one pass.
all_deps :=



CC.target ?= $(CC)
CFLAGS.target ?= $(CFLAGS)
CXX.target ?= $(CXX)
CXXFLAGS.target ?= $(CXXFLAGS)
LINK.target ?= $(LINK)
LDFLAGS.target ?= $(LDFLAGS)
AR.target ?= $(AR)

# C++ apps need to be linked with g++.
#
# Note: flock is used to seralize linking. Linking is a memory-intensive
# process so running parallel links can often lead to thrashing.  To disable
# the serialization, override LINK via an envrionment variable as follows:
#
#   export LINK=g++
#
# This will allow make to invoke N linker processes as specified in -jN.
LINK ?= flock $(builddir)/linker.lock $(CXX.target)

# TODO(evan): move all cross-compilation logic to gyp-time so we don't need
# to replicate this environment fallback in make as well.
CC.host ?= gcc
CFLAGS.host ?=
CXX.host ?= g++
CXXFLAGS.host ?=
LINK.host ?= $(CXX.host)
LDFLAGS.host ?=
AR.host ?= ar

# Define a dir function that can handle spaces.
# http://www.gnu.org/software/make/manual/make.html#Syntax-of-Functions
# "leading spaces cannot appear in the text of the first argument as written.
# These characters can be put into the argument value by variable substitution."
empty :=
space := $(empty) $(empty)

# http://stackoverflow.com/questions/1189781/using-make-dir-or-notdir-on-a-path-with-spaces
replace_spaces = $(subst $(space),?,$1)
unreplace_spaces = $(subst ?,$(space),$1)
dirx = $(call unreplace_spaces,$(dir $(call replace_spaces,$1)))

# Flags to make gcc output dependency info.  Note that you need to be
# careful here to use the flags that ccache and distcc can understand.
# We write to a dep file on the side first and then rename at the end
# so we can't end up with a broken dep file.
depfile = $(depsdir)/$(call replace_spaces,$@).d
DEPFLAGS = -MMD -MF $(depfile).raw

# We have to fixup the deps output in a few ways.
# (1) the file output should mention the proper .o file.
# ccache or distcc lose the path to the target, so we convert a rule of
# the form:
#   foobar.o: DEP1 DEP2
# into
#   path/to/foobar.o: DEP1 DEP2
# (2) we want missing files not to cause us to fail to build.
# We want to rewrite
#   foobar.o: DEP1 DEP2 \
#               DEP3
# to
#   DEP1:
#   DEP2:
#   DEP3:
# so if the files are missing, they're just considered phony rules.
# We have to do some pretty insane escaping to get those backslashes
# and dollar signs past make, the shell, and sed at the same time.
# Doesn't work with spaces, but that's fine: .d files have spaces in
# their names replaced with other characters.
define fixup_dep
# The depfile may not exist if the input file didn't have any #includes.
touch $(depfile).raw
# Fixup path as in (1).
sed -e "s|^$(notdir $@)|$@|" $(depfile).raw >> $(depfile)
# Add extra rules as in (2).
# We remove slashes and replace spaces with new lines;
# remove blank lines;
# delete the first line and append a colon to the remaining lines.
sed -e 's|\\||' -e 'y| |\n|' $(depfile).raw |\
  grep -v '^$$'                             |\
  sed -e 1d -e 's|$$|:|'                     \
    >> $(depfile)
rm $(depfile).raw
endef

# Command definitions:
# - cmd_foo is the actual command to run;
# - quiet_cmd_foo is the brief-output summary of the command.

quiet_cmd_cc = CC($(TOOLSET)) $@
cmd_cc = $(CC.$(TOOLSET)) $(GYP_CFLAGS) $(DEPFLAGS) $(CFLAGS.$(TOOLSET)) -c -o $@ $<

quiet_cmd_cxx = CXX($(TOOLSET)) $@
cmd_cxx = $(CXX.$(TOOLSET)) $(GYP_CXXFLAGS) $(DEPFLAGS) $(CXXFLAGS.$(TOOLSET)) -c -o $@ $<

quiet_cmd_touch = TOUCH $@
cmd_touch = touch $@

quiet_cmd_copy = COPY $@
# send stderr to /dev/null to ignore messages when linking directories.
cmd_copy = ln -f "$<" "$@" 2>/dev/null || (rm -rf "$@" && cp -af "$<" "$@")

quiet_cmd_alink = AR($(TOOLSET)) $@
cmd_alink = rm -f $@ && $(AR.$(TOOLSET)) crs $@ $(filter %.o,$^)

quiet_cmd_alink_thin = AR($(TOOLSET)) $@
cmd_alink_thin = rm -f $@ && $(AR.$(TOOLSET)) crsT $@ $(filter %.o,$^)

# Due to circular dependencies between libraries :(, we wrap the
# special "figure out circular dependencies" flags around the entire
# input list during linking.
quiet_cmd_link = LINK($(TOOLSET)) $@
cmd_link = $(LINK.$(TOOLSET)) $(GYP_LDFLAGS) $(LDFLAGS.$(TOOLSET)) -o $@ -Wl,--start-group $(LD_INPUTS) -Wl,--end-group $(LIBS)

# We support two kinds of shared objects (.so):
# 1) shared_library, which is just bundling together many dependent libraries
# into a link line.
# 2) loadable_module, which is generating a module intended for dlopen().
#
# They differ only slightly:
# In the former case, we want to package all dependent code into the .so.
# In the latter case, we want to package just the API exposed by the
# outermost module.
# This means shared_library uses --whole-archive, while loadable_module doesn't.
# (Note that --whole-archive is incompatible with the --start-group used in
# normal linking.)

# Other shared-object link notes:
# - Set SONAME to the library filename so our binaries don't reference
# the local, absolute paths used on the link command-line.
quiet_cmd_solink = SOLINK($(TOOLSET)) $@
cmd_solink = $(LINK.$(TOOLSET)) -shared $(GYP_LDFLAGS) $(LDFLAGS.$(TOOLSET)) -Wl,-soname=$(@F) -o $@ -Wl,--whole-archive $(LD_INPUTS) -Wl,--no-whole-archive $(LIBS)

quiet_cmd_solink_module = SOLINK_MODULE($(TOOLSET)) $@
cmd_solink_module = $(LINK.$(TOOLSET)) -shared $(GYP_LDFLAGS) $(LDFLAGS.$(TOOLSET)) -Wl,-soname=$(@F) -o $@ -Wl,--start-group $(filter-out FORCE_DO_CMD, $^) -Wl,--end-group $(LIBS)


# Define an escape_quotes function to escape single quotes.
# This allows us to handle quotes properly as long as we always use
# use single quotes and escape_quotes.
escape_quotes = $(subst ','\'',$(1))
# This comment is here just to include a ' to unconfuse syntax highlighting.
# Define an escape_vars function to escape '$' variable syntax.
# This allows us to read/write command lines with shell variables (e.g.
# $LD_LIBRARY_PATH), without triggering make substitution.
escape_vars = $(subst $$,$$$$,$(1))
# Helper that expands to a shell command to echo a string exactly as it is in
# make. This uses printf instead of echo because printf's behaviour with respect
# to escape sequences is more portable than echo's across different shells
# (e.g., dash, bash).
exact_echo = printf '%s\n' '$(call escape_quotes,$(1))'

# Helper to compare the command we're about to run against the command
# we logged the last time we ran the command.  Produces an empty
# string (false) when the commands match.
# Tricky point: Make has no string-equality test function.
# The kernel uses the following, but it seems like it would have false
# positives, where one string reordered its arguments.
#   arg_check = $(strip $(filter-out $(cmd_$(1)), $(cmd_$@)) \
#                       $(filter-out $(cmd_$@), $(cmd_$(1))))
# We instead substitute each for the empty string into the other, and
# say they're equal if both substitutions produce the empty string.
# .d files contain ? instead of spaces, take that into account.
command_changed = $(or $(subst $(cmd_$(1)),,$(cmd_$(call replace_spaces,$@))),\
                       $(subst $(cmd_$(call replace_spaces,$@)),,$(cmd_$(1))))

# Helper that is non-empty when a prerequisite changes.
# Normally make does this implicitly, but we force rules to always run
# so we can check their command lines.
#   $? -- new prerequisites
#   $| -- order-only dependencies
prereq_changed = $(filter-out FORCE_DO_CMD,$(filter-out $|,$?))

# Helper that executes all postbuilds until one fails.
define do_postbuilds
  @E=0;\
  for p in $(POSTBUILDS); do\
    eval $$p;\
    E=$$?;\
    if [ $$E -ne 0 ]; then\
      break;\
    fi;\
  done;\
  if [ $$E -ne 0 ]; then\
    rm -rf "$@";\
    exit $$E;\
  fi
endef

# do_cmd: run a command via the above cmd_foo names, if necessary.
# Should always run for a given target to handle command-line changes.
# Second argument, if non-zero, makes it do asm/C/C++ dependency munging.
# Third argument, if non-zero, makes it do POSTBUILDS processing.
# Note: We intentionally do NOT call dirx for depfile, since it contains ? for
# spaces already and dirx strips the ? characters.
define do_cmd
$(if $(or $(command_changed),$(prereq_changed)),
  @$(call exact_echo,  $($(quiet)cmd_$(1)))
  @mkdir -p "$(call dirx,$@)" "$(dir $(depfile))"
  $(if $(findstring flock,$(word 1,$(cmd_$1))),
    @$(cmd_$(1))
    @echo "  $(quiet_cmd_$(1)): Finished",
    @$(cmd_$(1))
  )
  @$(call exact_echo,$(call escape_vars,cmd_$(call replace_spaces,$@) := $(cmd_$(1)))) > $(depfile)
  @$(if $(2),$(fixup_dep))
  $(if $(and $(3), $(POSTBUILDS)),
    $(call do_postbuilds)
  )
)
endef

# Declare the "all" target first so it is the default,
# even though we don't have the deps yet.
.PHONY: all
all:

# make looks for ways to re-generate included makefiles, but in our case, we
# don't have a direct way. Explicitly telling make that it has nothing to do
# for them makes it go faster.
%.d: ;

# Use FORCE_DO_CMD to force a target to run.  Should be coupled with
# do_cmd.
.PHONY: FORCE_DO_CMD
FORCE_DO_CMD:

TOOLSET := host
# Suffix rules, putting all outputs into $(obj).
$(obj).$(TOOLSET)/%.o: $(srcdir)/%.c FORCE_DO_CMD
	@$(call do_cmd,cc,1)
$(obj).$(TOOLSET)/%.o: $(srcdir)/%.cc FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(srcdir)/%.cpp FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(srcdir)/%.cxx FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(srcdir)/%.S FORCE_DO_CMD
	@$(call do_cmd,cc,1)
$(obj).$(TOOLSET)/%.o: $(srcdir)/%.s FORCE_DO_CMD
	@$(call do_cmd,cc,1)

# Try building from generated source, too.
$(obj).$(TOOLSET)/%.o: $(obj).$(TOOLSET)/%.c FORCE_DO_CMD
	@$(call do_cmd,cc,1)
$(obj).$(TOOLSET)/%.o: $(obj).$(TOOLSET)/%.cc FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(obj).$(TOOLSET)/%.cpp FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(obj).$(TOOLSET)/%.cxx FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(obj).$(TOOLSET)/%.S FORCE_DO_CMD
	@$(call do_cmd,cc,1)
$(obj).$(TOOLSET)/%.o: $(obj).$(TOOLSET)/%.s FORCE_DO_CMD
	@$(call do_cmd,cc,1)

$(obj).$(TOOLSET)/%.o: $(obj)/%.c FORCE_DO_CMD
	@$(call do_cmd,cc,1)
$(obj).$(TOOLSET)/%.o: $(obj)/%.cc FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(obj)/%.cpp FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(obj)/%.cxx FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(obj)/%.S FORCE_DO_CMD
	@$(call do_cmd,cc,1)
$(obj).$(TOOLSET)/%.o: $(obj)/%.s FORCE_DO_CMD
	@$(call do_cmd,cc,1)

TOOLSET := target
# Suffix rules, putting all outputs into $(obj).
$(obj).$(TOOLSET)/%.o: $(srcdir)/%.c FORCE_DO_CMD
	@$(call do_cmd,cc,1)
$(obj).$(TOOLSET)/%.o: $(srcdir)/%.cc FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(srcdir)/%.cpp FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(srcdir)/%.cxx FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(srcdir)/%.S FORCE_DO_CMD
	@$(call do_cmd,cc,1)
$(obj).$(TOOLSET)/%.o: $(srcdir)/%.s FORCE_DO_CMD
	@$(call do_cmd,cc,1)

# Try building from generated source, too.
$(obj).$(TOOLSET)/%.o: $(obj).$(TOOLSET)/%.c FORCE_DO_CMD
	@$(call do_cmd,cc,1)
$(obj).$(TOOLSET)/%.o: $(obj).$(TOOLSET)/%.cc FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(obj).$(TOOLSET)/%.cpp FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(obj).$(TOOLSET)/%.cxx FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(obj).$(TOOLSET)/%.S FORCE_DO_CMD
	@$(call do_cmd,cc,1)
$(obj).$(TOOLSET)/%.o: $(obj).$(TOOLSET)/%.s FORCE_DO_CMD
	@$(call do_cmd,cc,1)

$(obj).$(TOOLSET)/%.o: $(obj)/%.c FORCE_DO_CMD
	@$(call do_cmd,cc,1)
$(obj).$(TOOLSET)/%.o: $(obj)/%.cc FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(obj)/%.cpp FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(obj)/%.cxx FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(obj)/%.S FORCE_DO_CMD
	@$(call do_cmd,cc,1)
$(obj).$(TOOLSET)/%.o: $(obj)/%.s FORCE_DO_CMD
	@$(call do_cmd,cc,1)


ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,All.target.mk)))),)
  include All.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/dbus.target.mk)))),)
  include build/linux/dbus.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/dridrm.target.mk)))),)
  include build/linux/dridrm.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/fontconfig.target.mk)))),)
  include build/linux/fontconfig.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/freetype2.target.mk)))),)
  include build/linux/freetype2.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/gconf.target.mk)))),)
  include build/linux/gconf.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/gdk.target.mk)))),)
  include build/linux/gdk.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/gio.target.mk)))),)
  include build/linux/gio.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/glib.host.mk)))),)
  include build/linux/glib.host.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/glib.target.mk)))),)
  include build/linux/glib.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/gnome_keyring.target.mk)))),)
  include build/linux/gnome_keyring.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/gnome_keyring_direct.target.mk)))),)
  include build/linux/gnome_keyring_direct.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/gtk.host.mk)))),)
  include build/linux/gtk.host.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/gtk.target.mk)))),)
  include build/linux/gtk.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/gtkprint.target.mk)))),)
  include build/linux/gtkprint.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/libbrlapi.target.mk)))),)
  include build/linux/libbrlapi.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/libcap.target.mk)))),)
  include build/linux/libcap.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/libgcrypt.target.mk)))),)
  include build/linux/libgcrypt.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/libpci.target.mk)))),)
  include build/linux/libpci.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/libresolv.target.mk)))),)
  include build/linux/libresolv.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/libspeechd.target.mk)))),)
  include build/linux/libspeechd.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/pangocairo.host.mk)))),)
  include build/linux/pangocairo.host.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/pangocairo.target.mk)))),)
  include build/linux/pangocairo.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/ssl.target.mk)))),)
  include build/linux/ssl.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/udev.target.mk)))),)
  include build/linux/udev.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/x11.host.mk)))),)
  include build/linux/x11.host.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/x11.target.mk)))),)
  include build/linux/x11.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/xcomposite.target.mk)))),)
  include build/linux/xcomposite.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/xcursor.target.mk)))),)
  include build/linux/xcursor.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/xdamage.target.mk)))),)
  include build/linux/xdamage.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/xext.target.mk)))),)
  include build/linux/xext.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/xfixes.target.mk)))),)
  include build/linux/xfixes.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/xi.target.mk)))),)
  include build/linux/xi.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/xrandr.host.mk)))),)
  include build/linux/xrandr.host.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/xrandr.target.mk)))),)
  include build/linux/xrandr.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/xrender.target.mk)))),)
  include build/linux/xrender.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,build/linux/xtst.target.mk)))),)
  include build/linux/xtst.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,net/third_party/nss/libssl.target.mk)))),)
  include net/third_party/nss/libssl.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/gunit.target.mk)))),)
  include talk/gunit.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/libjingle.target.mk)))),)
  include talk/libjingle.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/libjingle_media.target.mk)))),)
  include talk/libjingle_media.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/libjingle_media_unittest.target.mk)))),)
  include talk/libjingle_media_unittest.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/libjingle_media_unittest_run.target.mk)))),)
  include talk/libjingle_media_unittest_run.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/libjingle_p2p.target.mk)))),)
  include talk/libjingle_p2p.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/libjingle_p2p_unittest.target.mk)))),)
  include talk/libjingle_p2p_unittest.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/libjingle_p2p_unittest_run.target.mk)))),)
  include talk/libjingle_p2p_unittest_run.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/libjingle_peerconnection.target.mk)))),)
  include talk/libjingle_peerconnection.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/libjingle_peerconnection_jar.target.mk)))),)
  include talk/libjingle_peerconnection_jar.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/libjingle_peerconnection_java_unittest.target.mk)))),)
  include talk/libjingle_peerconnection_java_unittest.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/libjingle_peerconnection_so.target.mk)))),)
  include talk/libjingle_peerconnection_so.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/libjingle_peerconnection_test_jar.target.mk)))),)
  include talk/libjingle_peerconnection_test_jar.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/libjingle_peerconnection_unittest.target.mk)))),)
  include talk/libjingle_peerconnection_unittest.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/libjingle_peerconnection_unittest_run.target.mk)))),)
  include talk/libjingle_peerconnection_unittest_run.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/libjingle_sound.target.mk)))),)
  include talk/libjingle_sound.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/libjingle_sound_unittest.target.mk)))),)
  include talk/libjingle_sound_unittest.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/libjingle_sound_unittest_run.target.mk)))),)
  include talk/libjingle_sound_unittest_run.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/libjingle_unittest.target.mk)))),)
  include talk/libjingle_unittest.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/libjingle_unittest_main.target.mk)))),)
  include talk/libjingle_unittest_main.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/libjingle_unittest_run.target.mk)))),)
  include talk/libjingle_unittest_run.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/libjingle_xmpphelp.target.mk)))),)
  include talk/libjingle_xmpphelp.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/login.target.mk)))),)
  include talk/login.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/peerconnection_client.target.mk)))),)
  include talk/peerconnection_client.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/peerconnection_server.target.mk)))),)
  include talk/peerconnection_server.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/relayserver.target.mk)))),)
  include talk/relayserver.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/stunserver.target.mk)))),)
  include talk/stunserver.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,talk/turnserver.target.mk)))),)
  include talk/turnserver.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,testing/gmock.target.mk)))),)
  include testing/gmock.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,testing/gmock_main.target.mk)))),)
  include testing/gmock_main.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,testing/gtest.target.mk)))),)
  include testing/gtest.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,testing/gtest_main.target.mk)))),)
  include testing/gtest_main.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,testing/gtest_prod.host.mk)))),)
  include testing/gtest_prod.host.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,testing/gtest_prod.target.mk)))),)
  include testing/gtest_prod.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/expat/expat.target.mk)))),)
  include third_party/expat/expat.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/gflags/gflags.target.mk)))),)
  include third_party/gflags/gflags.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/icu/icudata.host.mk)))),)
  include third_party/icu/icudata.host.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/icu/icudata.target.mk)))),)
  include third_party/icu/icudata.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/icu/icui18n.host.mk)))),)
  include third_party/icu/icui18n.host.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/icu/icui18n.target.mk)))),)
  include third_party/icu/icui18n.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/icu/icuuc.host.mk)))),)
  include third_party/icu/icuuc.host.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/icu/icuuc.target.mk)))),)
  include third_party/icu/icuuc.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/jsoncpp/jsoncpp.target.mk)))),)
  include third_party/jsoncpp/jsoncpp.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libjpeg_turbo/libjpeg.target.mk)))),)
  include third_party/libjpeg_turbo/libjpeg.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libsrtp/libsrtp.target.mk)))),)
  include third_party/libsrtp/libsrtp.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libsrtp/rdbx_driver.target.mk)))),)
  include third_party/libsrtp/rdbx_driver.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libsrtp/replay_driver.target.mk)))),)
  include third_party/libsrtp/replay_driver.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libsrtp/roc_driver.target.mk)))),)
  include third_party/libsrtp/roc_driver.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libsrtp/rtpw.target.mk)))),)
  include third_party/libsrtp/rtpw.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libsrtp/srtp_driver.target.mk)))),)
  include third_party/libsrtp/srtp_driver.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libsrtp/srtp_runtest.target.mk)))),)
  include third_party/libsrtp/srtp_runtest.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libsrtp/srtp_test_aes_calc.target.mk)))),)
  include third_party/libsrtp/srtp_test_aes_calc.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libsrtp/srtp_test_cipher_driver.target.mk)))),)
  include third_party/libsrtp/srtp_test_cipher_driver.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libsrtp/srtp_test_datatypes_driver.target.mk)))),)
  include third_party/libsrtp/srtp_test_datatypes_driver.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libsrtp/srtp_test_env.target.mk)))),)
  include third_party/libsrtp/srtp_test_env.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libsrtp/srtp_test_kernel_driver.target.mk)))),)
  include third_party/libsrtp/srtp_test_kernel_driver.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libsrtp/srtp_test_rand_gen.target.mk)))),)
  include third_party/libsrtp/srtp_test_rand_gen.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libsrtp/srtp_test_sha1_driver.target.mk)))),)
  include third_party/libsrtp/srtp_test_sha1_driver.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libsrtp/srtp_test_stat_driver.target.mk)))),)
  include third_party/libsrtp/srtp_test_stat_driver.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libvpx/gen_asm_offsets_vp8.target.mk)))),)
  include third_party/libvpx/gen_asm_offsets_vp8.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libvpx/gen_asm_offsets_vpx_scale.target.mk)))),)
  include third_party/libvpx/gen_asm_offsets_vpx_scale.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libvpx/libvpx.target.mk)))),)
  include third_party/libvpx/libvpx.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libvpx/libvpx_asm_offsets_vp8.target.mk)))),)
  include third_party/libvpx/libvpx_asm_offsets_vp8.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libvpx/libvpx_asm_offsets_vpx_scale.target.mk)))),)
  include third_party/libvpx/libvpx_asm_offsets_vpx_scale.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libvpx/libvpx_intrinsics_mmx.target.mk)))),)
  include third_party/libvpx/libvpx_intrinsics_mmx.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libvpx/libvpx_intrinsics_sse2.target.mk)))),)
  include third_party/libvpx/libvpx_intrinsics_sse2.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libvpx/libvpx_intrinsics_ssse3.target.mk)))),)
  include third_party/libvpx/libvpx_intrinsics_ssse3.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libvpx/libvpx_obj_int_extract.host.mk)))),)
  include third_party/libvpx/libvpx_obj_int_extract.host.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/libyuv/libyuv.target.mk)))),)
  include third_party/libyuv/libyuv.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/opus/opus.target.mk)))),)
  include third_party/opus/opus.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/opus/opus_demo.target.mk)))),)
  include third_party/opus/opus_demo.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/protobuf/protobuf_full_do_not_use.host.mk)))),)
  include third_party/protobuf/protobuf_full_do_not_use.host.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/protobuf/protobuf_full_do_not_use.target.mk)))),)
  include third_party/protobuf/protobuf_full_do_not_use.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/protobuf/protobuf_lite.host.mk)))),)
  include third_party/protobuf/protobuf_lite.host.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/protobuf/protobuf_lite.target.mk)))),)
  include third_party/protobuf/protobuf_lite.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/protobuf/protoc.host.mk)))),)
  include third_party/protobuf/protoc.host.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/protobuf/py_proto.target.mk)))),)
  include third_party/protobuf/py_proto.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/usrsctp/usrsctplib.target.mk)))),)
  include third_party/usrsctp/usrsctplib.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/yasm/config_sources.host.mk)))),)
  include third_party/yasm/config_sources.host.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/yasm/generate_files.host.mk)))),)
  include third_party/yasm/generate_files.host.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/yasm/genmacro.host.mk)))),)
  include third_party/yasm/genmacro.host.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/yasm/genmodule.host.mk)))),)
  include third_party/yasm/genmodule.host.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/yasm/genperf.host.mk)))),)
  include third_party/yasm/genperf.host.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/yasm/genperf_libs.host.mk)))),)
  include third_party/yasm/genperf_libs.host.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/yasm/genstring.host.mk)))),)
  include third_party/yasm/genstring.host.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/yasm/genversion.host.mk)))),)
  include third_party/yasm/genversion.host.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/yasm/re2c.host.mk)))),)
  include third_party/yasm/re2c.host.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,third_party/yasm/yasm.host.mk)))),)
  include third_party/yasm/yasm.host.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/common_audio/common_audio.target.mk)))),)
  include webrtc/common_audio/common_audio.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/common_audio/common_audio_sse2.target.mk)))),)
  include webrtc/common_audio/common_audio_sse2.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/common_audio/common_audio_unittests.target.mk)))),)
  include webrtc/common_audio/common_audio_unittests.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/common_audio/common_audio_unittests_run.target.mk)))),)
  include webrtc/common_audio/common_audio_unittests_run.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/common_video/common_video.target.mk)))),)
  include webrtc/common_video/common_video.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/common_video/common_video_unittests.target.mk)))),)
  include webrtc/common_video/common_video_unittests.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/common_video/common_video_unittests_run.target.mk)))),)
  include webrtc/common_video/common_video_unittests_run.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/CNG.target.mk)))),)
  include webrtc/modules/CNG.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/G711.target.mk)))),)
  include webrtc/modules/G711.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/G722.target.mk)))),)
  include webrtc/modules/G722.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/G722Test.target.mk)))),)
  include webrtc/modules/G722Test.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/NetEq.target.mk)))),)
  include webrtc/modules/NetEq.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/NetEq4.target.mk)))),)
  include webrtc/modules/NetEq4.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/NetEq4TestTools.target.mk)))),)
  include webrtc/modules/NetEq4TestTools.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/NetEqRTPplay.target.mk)))),)
  include webrtc/modules/NetEqRTPplay.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/NetEqTestTools.target.mk)))),)
  include webrtc/modules/NetEqTestTools.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/PCM16B.target.mk)))),)
  include webrtc/modules/PCM16B.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/RTPanalyze.target.mk)))),)
  include webrtc/modules/RTPanalyze.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/RTPcat.target.mk)))),)
  include webrtc/modules/RTPcat.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/RTPchange.target.mk)))),)
  include webrtc/modules/RTPchange.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/RTPencode.target.mk)))),)
  include webrtc/modules/RTPencode.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/RTPjitter.target.mk)))),)
  include webrtc/modules/RTPjitter.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/RTPtimeshift.target.mk)))),)
  include webrtc/modules/RTPtimeshift.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/acm2.target.mk)))),)
  include webrtc/modules/acm2.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/audio_coding_module.target.mk)))),)
  include webrtc/modules/audio_coding_module.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/audio_conference_mixer.target.mk)))),)
  include webrtc/modules/audio_conference_mixer.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/audio_decoder_unittests.target.mk)))),)
  include webrtc/modules/audio_decoder_unittests.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/audio_decoder_unittests_run.target.mk)))),)
  include webrtc/modules/audio_decoder_unittests_run.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/audio_device.target.mk)))),)
  include webrtc/modules/audio_device.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/audio_device_test_func.target.mk)))),)
  include webrtc/modules/audio_device_test_func.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/audio_device_tests.target.mk)))),)
  include webrtc/modules/audio_device_tests.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/audio_device_tests_run.target.mk)))),)
  include webrtc/modules/audio_device_tests_run.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/audio_processing.target.mk)))),)
  include webrtc/modules/audio_processing.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/audio_processing_sse2.target.mk)))),)
  include webrtc/modules/audio_processing_sse2.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/audioproc.target.mk)))),)
  include webrtc/modules/audioproc.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/audioproc_debug_proto.target.mk)))),)
  include webrtc/modules/audioproc_debug_proto.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/audioproc_unittest_proto.target.mk)))),)
  include webrtc/modules/audioproc_unittest_proto.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/bitrate_controller.target.mk)))),)
  include webrtc/modules/bitrate_controller.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/bwe_rtp_play.target.mk)))),)
  include webrtc/modules/bwe_rtp_play.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/bwe_rtp_to_text.target.mk)))),)
  include webrtc/modules/bwe_rtp_to_text.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/bwe_tools_util.target.mk)))),)
  include webrtc/modules/bwe_tools_util.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/delay_test.target.mk)))),)
  include webrtc/modules/delay_test.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/desktop_capture.target.mk)))),)
  include webrtc/modules/desktop_capture.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/desktop_capture_differ_sse2.target.mk)))),)
  include webrtc/modules/desktop_capture_differ_sse2.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/g711_test.target.mk)))),)
  include webrtc/modules/g711_test.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/iLBC.target.mk)))),)
  include webrtc/modules/iLBC.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/iLBCtest.target.mk)))),)
  include webrtc/modules/iLBCtest.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/iSAC.target.mk)))),)
  include webrtc/modules/iSAC.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/iSACAPITest.target.mk)))),)
  include webrtc/modules/iSACAPITest.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/iSACFix.target.mk)))),)
  include webrtc/modules/iSACFix.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/iSACFixtest.target.mk)))),)
  include webrtc/modules/iSACFixtest.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/iSACSwitchSampRateTest.target.mk)))),)
  include webrtc/modules/iSACSwitchSampRateTest.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/iSACtest.target.mk)))),)
  include webrtc/modules/iSACtest.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/insert_packet_with_timing.target.mk)))),)
  include webrtc/modules/insert_packet_with_timing.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/media_file.target.mk)))),)
  include webrtc/modules/media_file.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/modules_tests.target.mk)))),)
  include webrtc/modules/modules_tests.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/modules_tests_run.target.mk)))),)
  include webrtc/modules/modules_tests_run.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/modules_unittests.target.mk)))),)
  include webrtc/modules/modules_unittests.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/modules_unittests_run.target.mk)))),)
  include webrtc/modules/modules_unittests_run.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/neteq3_speed_test.target.mk)))),)
  include webrtc/modules/neteq3_speed_test.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/neteq4_speed_test.target.mk)))),)
  include webrtc/modules/neteq4_speed_test.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/neteq_rtpplay.target.mk)))),)
  include webrtc/modules/neteq_rtpplay.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/neteq_unittest_tools.target.mk)))),)
  include webrtc/modules/neteq_unittest_tools.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/neteq_unittests.target.mk)))),)
  include webrtc/modules/neteq_unittests.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/neteq_unittests_run.target.mk)))),)
  include webrtc/modules/neteq_unittests_run.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/paced_sender.target.mk)))),)
  include webrtc/modules/paced_sender.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/remote_bitrate_estimator.target.mk)))),)
  include webrtc/modules/remote_bitrate_estimator.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/remote_bitrate_estimator/rbe_components.target.mk)))),)
  include webrtc/modules/remote_bitrate_estimator/rbe_components.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/rtp_rtcp.target.mk)))),)
  include webrtc/modules/rtp_rtcp.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/rtp_to_text.target.mk)))),)
  include webrtc/modules/rtp_to_text.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/test_framework.target.mk)))),)
  include webrtc/modules/test_framework.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/test_packet_masks_metrics.target.mk)))),)
  include webrtc/modules/test_packet_masks_metrics.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/unpack_aecdump.target.mk)))),)
  include webrtc/modules/unpack_aecdump.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/video_capture_module.target.mk)))),)
  include webrtc/modules/video_capture_module.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/video_capture_tests.target.mk)))),)
  include webrtc/modules/video_capture_tests.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/video_capture_tests_run.target.mk)))),)
  include webrtc/modules/video_capture_tests_run.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/video_codecs_test_framework.target.mk)))),)
  include webrtc/modules/video_codecs_test_framework.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/video_coding/codecs/vp8/vp8_coder.target.mk)))),)
  include webrtc/modules/video_coding/codecs/vp8/vp8_coder.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/video_coding/codecs/vp8/webrtc_vp8.target.mk)))),)
  include webrtc/modules/video_coding/codecs/vp8/webrtc_vp8.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/video_coding/utility/video_coding_utility.target.mk)))),)
  include webrtc/modules/video_coding/utility/video_coding_utility.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/video_coding_test.target.mk)))),)
  include webrtc/modules/video_coding_test.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/video_processing.target.mk)))),)
  include webrtc/modules/video_processing.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/video_processing_sse2.target.mk)))),)
  include webrtc/modules/video_processing_sse2.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/video_quality_measurement.target.mk)))),)
  include webrtc/modules/video_quality_measurement.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/video_render_module.target.mk)))),)
  include webrtc/modules/video_render_module.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/video_render_tests.target.mk)))),)
  include webrtc/modules/video_render_tests.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/video_render_tests_run.target.mk)))),)
  include webrtc/modules/video_render_tests_run.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/webrtc_i420.target.mk)))),)
  include webrtc/modules/webrtc_i420.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/webrtc_opus.target.mk)))),)
  include webrtc/modules/webrtc_opus.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/webrtc_utility.target.mk)))),)
  include webrtc/modules/webrtc_utility.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/modules/webrtc_video_coding.target.mk)))),)
  include webrtc/modules/webrtc_video_coding.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/system_wrappers/source/system_wrappers.target.mk)))),)
  include webrtc/system_wrappers/source/system_wrappers.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/system_wrappers/source/system_wrappers_unittests.target.mk)))),)
  include webrtc/system_wrappers/source/system_wrappers_unittests.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/system_wrappers/source/system_wrappers_unittests_run.target.mk)))),)
  include webrtc/system_wrappers/source/system_wrappers_unittests_run.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/test/buildbot_tests_scripts.target.mk)))),)
  include webrtc/test/buildbot_tests_scripts.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/test/channel_transport.target.mk)))),)
  include webrtc/test/channel_transport.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/test/frame_generator.target.mk)))),)
  include webrtc/test/frame_generator.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/test/libtest/libtest.target.mk)))),)
  include webrtc/test/libtest/libtest.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/test/metrics.target.mk)))),)
  include webrtc/test/metrics.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/test/test_support.target.mk)))),)
  include webrtc/test/test_support.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/test/test_support_main.target.mk)))),)
  include webrtc/test/test_support_main.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/test/test_support_main_threaded_mac.target.mk)))),)
  include webrtc/test/test_support_main_threaded_mac.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/test/test_support_unittests.target.mk)))),)
  include webrtc/test/test_support_unittests.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/test/test_support_unittests_run.target.mk)))),)
  include webrtc/test/test_support_unittests_run.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/test/webrtc_test_common.target.mk)))),)
  include webrtc/test/webrtc_test_common.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/test/webrtc_test_common_unittests.target.mk)))),)
  include webrtc/test/webrtc_test_common_unittests.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/tools/audio_e2e_harness.target.mk)))),)
  include webrtc/tools/audio_e2e_harness.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/tools/command_line_parser.target.mk)))),)
  include webrtc/tools/command_line_parser.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/tools/force_mic_volume_max.target.mk)))),)
  include webrtc/tools/force_mic_volume_max.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/tools/frame_analyzer.target.mk)))),)
  include webrtc/tools/frame_analyzer.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/tools/frame_editing_lib.target.mk)))),)
  include webrtc/tools/frame_editing_lib.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/tools/frame_editor.target.mk)))),)
  include webrtc/tools/frame_editor.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/tools/psnr_ssim_analyzer.target.mk)))),)
  include webrtc/tools/psnr_ssim_analyzer.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/tools/rgba_to_i420_converter.target.mk)))),)
  include webrtc/tools/rgba_to_i420_converter.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/tools/tools_unittests.target.mk)))),)
  include webrtc/tools/tools_unittests.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/tools/tools_unittests_run.target.mk)))),)
  include webrtc/tools/tools_unittests_run.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/tools/video_quality_analysis.target.mk)))),)
  include webrtc/tools/video_quality_analysis.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/video_engine/libvietest.target.mk)))),)
  include webrtc/video_engine/libvietest.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/video_engine/video_engine_core.target.mk)))),)
  include webrtc/video_engine/video_engine_core.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/video_engine/video_engine_core_unittests.target.mk)))),)
  include webrtc/video_engine/video_engine_core_unittests.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/video_engine/video_engine_core_unittests_run.target.mk)))),)
  include webrtc/video_engine/video_engine_core_unittests_run.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/video_engine/vie_auto_test.target.mk)))),)
  include webrtc/video_engine/vie_auto_test.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/video_engine/vie_auto_test_run.target.mk)))),)
  include webrtc/video_engine/vie_auto_test_run.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/video_engine_tests.target.mk)))),)
  include webrtc/video_engine_tests.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/video_engine_tests_run.target.mk)))),)
  include webrtc/video_engine_tests_run.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/video_loopback.target.mk)))),)
  include webrtc/video_loopback.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/voice_engine/voe_auto_test.target.mk)))),)
  include webrtc/voice_engine/voe_auto_test.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/voice_engine/voe_auto_test_run.target.mk)))),)
  include webrtc/voice_engine/voe_auto_test_run.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/voice_engine/voe_cmd_test.target.mk)))),)
  include webrtc/voice_engine/voe_cmd_test.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/voice_engine/voice_engine.target.mk)))),)
  include webrtc/voice_engine/voice_engine.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/voice_engine/voice_engine_unittests.target.mk)))),)
  include webrtc/voice_engine/voice_engine_unittests.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/voice_engine/voice_engine_unittests_run.target.mk)))),)
  include webrtc/voice_engine/voice_engine_unittests_run.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/webrtc.target.mk)))),)
  include webrtc/webrtc.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/webrtc_all.target.mk)))),)
  include webrtc/webrtc_all.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/webrtc_perf_tests.target.mk)))),)
  include webrtc/webrtc_perf_tests.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/webrtc_perf_tests_run.target.mk)))),)
  include webrtc/webrtc_perf_tests_run.target.mk
endif
ifeq ($(strip $(foreach prefix,$(NO_LOAD),\
    $(findstring $(join ^,$(prefix)),\
                 $(join ^,webrtc/webrtc_tests.target.mk)))),)
  include webrtc/webrtc_tests.target.mk
endif

quiet_cmd_regen_makefile = ACTION Regenerating $@
cmd_regen_makefile = cd $(srcdir); ./webrtc/build/gyp_webrtc -fmake --ignore-environment "--toplevel-dir=." -I/home/lht/webrtc/trunk/build/common.gypi -I/home/lht/webrtc/trunk/webrtc/supplement.gypi "--depth=." --check "-Dextra_gyp_flag=0" all.gyp
Makefile: webrtc/tools/tools.gyp third_party/expat/expat.gyp webrtc/modules/audio_device/audio_device.gypi third_party/opus/opus_srcs.gypi webrtc/modules/audio_coding/codecs/opus/opus.gypi third_party/icu/icu.gyp webrtc/modules/rtp_rtcp/test/testFec/test_fec.gypi webrtc/test/metrics.gyp third_party/libvpx/obj_int_extract.gypi third_party/libvpx/libvpx_srcs_x86_intrinsics.gypi net/third_party/nss/ssl.gyp webrtc/test/libtest/libtest.gyp build/internal/release_impl_official.gypi build/ios/mac_build.gypi webrtc/modules/modules_tests.isolate webrtc/modules/audio_coding/neteq4/neteq_tests.gypi build/android/instr_action.gypi third_party/libvpx/libvpx_srcs_arm_neon_cpu_detect.gypi webrtc/video_engine/test/auto_test/vie_auto_test.isolate talk/libjingle_unittest.isolate testing/gtest.gyp webrtc/test/webrtc_test_common.gyp third_party/libyuv/libyuv.gyp webrtc/modules/audio_coding/codecs/pcm16b/pcm16b.gypi webrtc/common_video/common_video_unittests.gyp webrtc/video_engine/test/libvietest/libvietest.gypi third_party/libvpx/libvpx_srcs_x86.gypi third_party/opus/opus.gyp third_party/libvpx/libvpx_srcs_mips.gypi webrtc/common_video/common_video.gyp webrtc/build/isolate.gypi third_party/protobuf/protobuf.gyp webrtc/voice_engine/voice_engine_unittests.isolate webrtc/video_engine/test/auto_test/vie_auto_test.gypi third_party/libvpx/libvpx_srcs_arm_neon_cpu_detect_intrinsics.gypi webrtc/system_wrappers/source/system_wrappers_unittests.isolate talk/libjingle.gyp webrtc/webrtc.gyp webrtc/modules/audio_processing/audio_processing.gypi webrtc/modules/video_coding/main/source/video_coding.gypi webrtc/modules/video_coding/codecs/test_framework/test_framework.gypi webrtc/modules/pacing/pacing.gypi webrtc/tools/internal_tools.gyp webrtc/build/common.gypi webrtc/system_wrappers/source/system_wrappers_tests.gyp webrtc/video_engine_tests.isolate third_party/usrsctp/usrsctp.gyp webrtc/modules/video_processing/main/source/video_processing.gypi third_party/yasm/yasm_compile.gypi all.gyp third_party/yasm/yasm.gyp talk/libjingle_p2p_unittest.isolate build/release.gypi webrtc/modules/video_coding/codecs/test/video_codecs_test_framework.gypi webrtc/modules/audio_coding/codecs/g711/g711.gypi webrtc/build/arm_neon.gypi build/android/dex_action.gypi talk/libjingle_peerconnection_unittest.isolate webrtc/common_audio/common_audio_unittests.isolate third_party/protobuf/protobuf_lite.gypi talk/libjingle_tests.gyp webrtc/video_engine/video_engine.gyp webrtc/common_video/common_video_unittests.isolate webrtc/common_audio/common_audio.gyp build/grit_action.gypi webrtc/modules/video_capture/video_capture_tests.isolate webrtc/modules/bitrate_controller/bitrate_controller.gypi webrtc/modules/video_coding/codecs/tools/video_codecs_tools.gypi webrtc/modules/video_coding/main/source/video_coding_test.gypi webrtc/modules/utility/source/utility.gypi talk/libjingle_examples.gyp webrtc/modules/remote_bitrate_estimator/remote_bitrate_estimator.gypi third_party/libvpx/libvpx_srcs_x86_64_intrinsics.gypi build/internal/release_defaults.gypi webrtc/build/protoc.gypi build/shim_headers.gypi webrtc/modules/video_coding/codecs/vp8/vp8.gyp webrtc/modules/audio_coding/codecs/isac/isacfix_test.gypi third_party/libsrtp/libsrtp.gyp build/java.gypi webrtc/video_engine/video_engine_core.gypi third_party/libvpx/libvpx_srcs_arm_neon.gypi webrtc/modules/audio_coding/neteq4/neteq.gypi webrtc/modules/video_coding/codecs/i420/main/source/i420.gypi third_party/icu/icu.gypi webrtc/modules/audio_coding/neteq/neteq.gypi webrtc/modules/desktop_capture/desktop_capture.gypi third_party/libvpx/libvpx.gyp webrtc/modules/video_capture/video_capture.gypi webrtc/modules/modules.gyp webrtc/voice_engine/voe_auto_test.isolate webrtc/modules/audio_conference_mixer/source/audio_conference_mixer.gypi third_party/libjpeg_turbo/libjpeg.gyp webrtc/video_engine/video_engine_core_unittests.isolate talk/libjingle_media_unittest.isolate webrtc/webrtc_perf_tests.isolate webrtc/test/test_support_unittests.isolate webrtc/modules/modules_unittests.isolate webrtc/voice_engine/voice_engine.gyp third_party/libvpx/unpack_lib_posix.gypi testing/gmock.gyp third_party/libvpx/libvpx_srcs_arm.gypi build/filename_rules.gypi webrtc/modules/video_render/video_render.gypi third_party/jsoncpp/jsoncpp.gyp webrtc/modules/audio_processing/audio_processing_tests.gypi webrtc/supplement.gypi build/internal/release_impl.gypi webrtc/modules/audio_coding/neteq/neteq_unittests.isolate talk/libjingle_sound_unittest.isolate webrtc/webrtc_tests.gypi webrtc/modules/audio_coding/codecs/isac/main/source/isac.gypi talk/build/isolate.gypi third_party/libvpx/libvpx_srcs_x86_64.gypi webrtc/modules/audio_coding/main/acm2/audio_coding_module.gypi webrtc/tools/tools_unittests.isolate webrtc/test/test.gyp build/android/cpufeatures.gypi webrtc/modules/audio_device/audio_device_tests.isolate webrtc/modules/video_render/video_render_tests.isolate third_party/libyuv/libyuv.gypi webrtc/modules/audio_coding/codecs/ilbc/ilbc.gypi webrtc/video/webrtc_video.gypi build/common.gypi third_party/gflags/gflags.gyp webrtc/modules/audio_coding/neteq4/audio_decoder_unittests.isolate webrtc/system_wrappers/source/system_wrappers.gyp build/android/lint_action.gypi webrtc/modules/audio_coding/codecs/isac/isac_test.gypi webrtc/modules/rtp_rtcp/source/rtp_rtcp.gypi talk/build/common.gypi webrtc/modules/remote_bitrate_estimator/remote_bitrate_estimator_components.gyp webrtc/modules/media_file/source/media_file.gypi webrtc/modules/audio_coding/codecs/cng/cng.gypi webrtc/modules/audio_coding/codecs/g722/g722.gypi webrtc/modules/audio_coding/codecs/isac/fix/source/isacfix.gypi third_party/jsoncpp/jsoncpp.gypi build/linux/system.gyp webrtc/modules/audio_coding/main/source/audio_coding_module.gypi webrtc/modules/video_coding/utility/video_coding_utility.gyp
	$(call do_cmd,regen_makefile)

# "all" is a concatenation of the "all" targets from all the included
# sub-makefiles. This is just here to clarify.
all:

# Add in dependency-tracking rules.  $(all_deps) is the list of every single
# target in our tree. Only consider the ones with .d (dependency) info:
d_files := $(wildcard $(foreach f,$(all_deps),$(depsdir)/$(f).d))
ifneq ($(d_files),)
  include $(d_files)
endif
