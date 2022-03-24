#include <iostream>
#include <vector>
#include <thread>
#include <functional>
#include <utility>
#include <cmath>

//умножает вектор на строку из косинусов
double product(const std::vector<double>& v, int i) 
{
    int n=v.size();
    double res=0.0;
    for (int j=0; j<n; ++j)
    {
        res+=cos((i*j*M_PI)/(n-1))*v[j];
    }
    return res;
}

void compute(const std::vector<double>& v,std::vector<double>& ans, int st, int end)
{
    for (int i=st; i<end; ++i)
    {
        ans[i]=product(v,i);
    }
}

void concur4(const std::vector<double>& v,std::vector<double>& ans)
{
    int n=v.size();
    ans.resize(n,0.0);
    std::thread th1(compute,std::ref(v),std::ref(ans),0,n/4);
    std::thread th2(compute,std::ref(v),std::ref(ans),n/4,n/2);
    std::thread th3(compute,std::ref(v),std::ref(ans),n/2,3*n/4);
    std::thread th4(compute,std::ref(v),std::ref(ans),3*n/4,n);
    th1.join();
    th2.join();
    th3.join();
    th4.join();
}

void concur2(const std::vector<double>& v,std::vector<double>& ans)
{
    int n=v.size();
    ans.resize(n,0.0);
    std::thread th1(compute,std::ref(v),std::ref(ans),0,n/2);
    std::thread th2(compute,std::ref(v),std::ref(ans),n/2,n);
    th1.join();
    th2.join();
}

struct rgb
{
  std::vector<double> red;
  std::vector<double> green;
  std::vector<double> blue;
  void dct()
  {
      std::vector<double> buf;
      concur2(red,buf);
      std::swap(red,buf);
      concur2(green,buf);
      std::swap(green,buf);
      concur2(blue,buf);
      std::swap(blue,buf);
  }
};

int main()
{
    std::vector<double> v={1,-1,1,-1}, ans;
    concur2(v,ans);
    for (auto& it: ans)
    {
        std::cout<<it<<" ";
    }
    return 0;
}
