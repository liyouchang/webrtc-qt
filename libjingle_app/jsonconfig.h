#ifndef JSONCONFIG_H
#define JSONCONFIG_H

#include "talk/base/json.h"

extern const char kConfigFileName[];

class JsonConfig
{
public:
    JsonConfig();

    // 添加配置
    // @key_path: Json的Path查找路径
    // @value: 设置的值
    // return: 成功/失败
    template<typename T>
    bool Set(const std::string& key_path, const T& value) {
        std::string::size_type pos = key_path.find_last_of('.');
        if(pos != std::string::npos){
            std::string name = key_path.substr(pos+1, key_path.size()-(pos+1));
            Json::Value& v = Construct(key_path.substr(0, pos+1));
            v[name] = value;
        }else{
            value_[key_path] = value;
        }
        return true;
    }

    // 获取指定路径的配置数据
    // @key_path: Json的Path查找路径
    // @value: 设置的值
    // return: 返回(类型为JsonValue)配置数据，可以是int, std::string,JsonArray 等等，
    //		   根据实际需求转换
    template<typename T>
    Json::Value Get(const std::string& key_path, const T& default_value) const
    {
        std::string s = key_path;
        s.append(1, '.');

        std::string::size_type cur = 0;
        std::string::size_type pos = s.find('.', cur);

        std::string name;
        Json::Value v;
        const Json::Value* t = &value_;
        do {
            name = s.substr(cur, pos-cur);
            cur = pos+1;
            if((*t)[name].isNull()) {
                return Json::Value(default_value);
            }
            t = &(*t)[name];
            pos = s.find('.', cur);
        } while(pos != std::string::npos);

        v = *t;
        return v;
    }

private:
    // 构造json路径
    Json::Value& Construct(const std::string& path) {
        std::string::size_type cur = 0;
        std::string::size_type pos = path.find('.', cur);
        std::string name;

        Json::Value* t = &value_;
        do {
            name = path.substr(cur, pos-cur);
            cur = pos+1;
            if((*t)[name].isNull()) {
                (*t)[name] = Json::Value(Json::objectValue);
            }
            t = &(*t)[name];
            pos = path.find('.', cur);
        } while(pos != std::string::npos);

        return *t;
    }

public:
    // 序列化,反序列化
    bool FromString(const std::string& in);
    std::string ToString();
    bool FromFile(const std::string& filename );
    bool ToFile(const std::string& sSaveFile);
    static JsonConfig *Instance();
private:
    Json::Value value_;
};

#endif // JSONCONFIG_H
