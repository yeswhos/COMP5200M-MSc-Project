#include <cstdlib>
#include <vector>

using namespace std;
using namespace BEAST;




class AverageDistAnimat : public Animat
{

private:

int interval;
vector<Item*> items;

public:

AverageDistAnimat(int intervals)
{
  interval = intervals;
  SetLocation(-50, -50);
}

virtual void Control()
{
  int timeSteps = GetTimeStep();
  cout << timeSteps << endl;;
  
  if (timeSteps % interval == 0)
  {
    //cout << timeSteps << endl;
  }
}

};
