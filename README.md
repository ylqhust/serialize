# c++ serialize, unserialize lib, header file only  
## support all pod type, char, unsigned char, bool, short, unsigned short, int ,unsigned int, long, unsigned long  
## long long, unsigned long long, float, double, std::string, std::wstring, std::vector, std::map
  

### sample code
```c++
#include <iostream>
#include "serialize.h"

int main()
{
    Serialize serialize;
    serialize << true;
    serialize << false;
    serialize << 1;
    serialize << 3.14f;
    serialize << 3.141592657;
    serialize << 'r';
    serialize << L'n';
    serialize << "g";
    serialize << L"edg";
    serialize << "";
    serialize << L"";
    std::vector<std::vector<int>> vector1 = { {1,2,3} };
    serialize << vector1;
    std::map<int, std::vector<int>> map1 = { {1,{1,2,3}}, {2,{4,5,6}} };
    serialize << map1;
    std::map<std::string, std::string> map2 = { {"name","alice"},{"age","10"} };
    serialize << map2;

    UnSerialize unserialize(serialize.buffer(), serialize.length());
    std::vector<Variant> vs;
    while (!unserialize.isEmpty())
    {
        Variant v;
        unserialize >> v;
        if (v.isValid())
            vs.push_back(v);
        else
            break;
    }
    bool v1 = vs[0].to<bool>();
    bool v2 = vs[1].to<bool>();
    int v3 = vs[2].to<int>();
    float v4 = vs[3].to<float>();
    double v5 = vs[4].to<double>();
    char v6 = vs[5].to<char>();
    wchar_t v7 = vs[6].to<wchar_t>();
    std::string v8 = vs[7].to<std::string>();
    std::wstring v9 = vs[8].to<std::wstring>();
    std::string v10 = vs[9].to<std::string>();
    std::wstring v11 = vs[10].to<std::wstring>();
    std::vector<std::vector<int>> v12;
    vs[11].to(v12);
    std::map<int, std::vector<int>> v13;
    vs[12].to(v13);
    std::map<std::string, std::string> v14;
    vs[13].to(v14);
    return 0;
}
```