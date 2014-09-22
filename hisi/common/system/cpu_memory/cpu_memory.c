#include "cpu_memory.h"

int cpuUsageRate=0;
int iSecondCnt05=0;
int iSecondCnt10=0;

int memUsageRate=0;
int iSecondMem05=0;
struct MemInfo mem;
struct OccupyCpu
{
    char name[20];
    unsigned int user;
    unsigned int nice;
    unsigned int system;
    unsigned int idle;
};
struct OccupyCpu ocpu;
struct OccupyCpu ncpu;

static char *skip_token(const char *p)
{
    while (isspace(*p))
    	p++;
    while (*p && !isspace(*p))
    	p++;

    return (char *)p;
}

void get_cpu_occupy(struct OccupyCpu * o)
{
    int fd;
    int len;
    char buff[1024];

    fd = open("/proc/stat", O_RDONLY);
    if (fd < 0) return;

    len = read(fd,buff,sizeof(buff)-1);
    close(fd);
    if (len < 30)
    	return;
    buff[len] = 0;
    sscanf (buff, "%s %u %u %u %u", o->name, &o->user, &o->nice, &o->system, &o->idle);
}

int cal_cpu(struct OccupyCpu *o, struct OccupyCpu *n)
{
    int od,nd;
    int id,sd;

    od = (o->user+o->nice+o->system+o->idle);
    nd = (n->user+n->nice+n->system+n->idle);
    id = (n->user-o->user);
    sd = (n->system-o->system);

    if (nd-od)
    	return 100-(n->idle-o->idle)*100/(nd-od);
    else
        return -1;
}

int g_iCpuHighCnt=0;
int cal_cpu_occupy (void)
{
    iSecondCnt05++;
    if (iSecondCnt05 >= 5)
    {
        iSecondCnt05 = 0;
        memcpy(&ocpu,&ncpu,sizeof(struct OccupyCpu));
        get_cpu_occupy(&ncpu);
    }
    iSecondCnt10++;
    if (iSecondCnt10 >= 10)
    {
        iSecondCnt10 = 0;
        cpuUsageRate = cal_cpu(&ocpu,&ncpu);
        printf("CPUUsed=%d%s MemoryUsed=%d%s MemoryTotal=%dM\n",cpuUsageRate,"%",memUsageRate,"%",mem.total/1024);
        if (cpuUsageRate >= 97)
        {
            g_iCpuHighCnt++;
            if (g_iCpuHighCnt > 3)
            {
                g_iCpuHighCnt = 0;
                system("killall thttpd");
            }
        }
        else
            g_iCpuHighCnt = 0;
    }
    return 0;
}

/***************************************************************************************/
int get_mem_occupy(struct MemInfo *mem)
{
    char buffer[2048];
    int fd;
    int len,memsize_valid;

    char *p;
    fd = open("/proc/meminfo", O_RDONLY);
    if (fd < 0) return -1;
    len = read(fd, buffer, sizeof(buffer)-1);
	close(fd);

	if (len <= 0)
		return -1;
	buffer[len] = '\0';
	p = buffer;
	p = skip_token(p);
	if (p!=0) mem->total = strtoul(p, &p, 10); // total memory

	if (p!=0) p = strchr(p, '\n');
	if (p!=0) p = skip_token(p);
	if (p!=0) mem->free = strtoul(p, &p, 10); // free memory

	if (p!=0) p = strchr(p, '\n');
	if (p!=0) p = skip_token(p);
	if (p!=0) mem->buffer = strtoul(p, &p, 10); //buffer memory

	if (p!=0) p = strchr(p, '\n');
	if (p!=0) p = skip_token(p);
	if (p!=0) mem->cached = strtoul(p, &p, 10); //cached memory

	mem->Dirty=0;
	if (p!=0) p = strstr(p, "Dirty");
	if (p!=0) p = skip_token(p);
	if (p!=0) mem->Dirty = strtoul(p, &p, 10); //Dirty memory


	if (mem->total)
	{
	    memsize_valid = mem->total -mem->free -mem->buffer -mem->cached +mem->Dirty;
		return memsize_valid*100/mem->total;
	}
    return -1;
}

int cal_mem_occupy(void)
{
    iSecondMem05++;
    if (iSecondMem05 >= 5)
    {
        iSecondMem05 = 0;
        memUsageRate = get_mem_occupy(&mem);
    }
    return 0;
}
