#ifndef _SITUATEDSENSORS_H
#define _SITUATEDSENSORS_H


using namespace std;
using namespace BEAST;


struct ScaleInverseArea : public SensorScaleFunction
{
  ScaleInverseArea(double area):inverseArea(1.0 / area){}

  virtual double operator()(double input)
  {
    return inverseArea * input;
  }

private:
  double inverseArea;
};


struct ScaleByAlpha : public SensorScaleFunction
{
  ScaleByAlpha(double a):alpha(a){}

  virtual double operator()(double input)
  {
    return input / alpha;
  }

private:
  double alpha;
};


struct NoScale : public SensorScaleFunction
{

  virtual double operator()(double input)
  {
    return input;
  }

};


class EvalColour : public SensorEvalFunction
{
public:
  EvalColour(int col):colour(col)
  {
    colourSum = 0.0; 
    itemCount = 0.0;
  }

  virtual void Reset()
  {
    colourSum = 0.0;
    itemCount = 0.0;
  }
    
  virtual void operator()(WorldObject* obj, const Vector2D& loc)
  {

    colourSum += obj->GetColour(colour);
    //cout << itemCount << endl;
    itemCount++;
  }
  
  double GetOutput()const
  {
    //cout << itemCount << endl;
    //cout << colourSum << endl;
    if (itemCount > 0)
      return colourSum / 3.0;
    else
      return 0.0;
  }


private:
  double colourSum;
  double itemCount;
  int colour;
};

/*
* For both EvalNearestXPerimeter and EvalNearestYPerimeter
* the world objects detected must be circular.
*
* Possible extensions here!
*/

class EvalNearestXPerimeter : public EvalNearest
{
public:
  EvalNearestXPerimeter(WorldObject* o, double range):EvalNearest(o, range){}

  virtual double GetOutput()const
  {
    float diff = bestCandidateVec.x - owner->GetLocation().x;

    
    if (diff >= 0.0)
      diff -= bestCandidate->GetRadius();
    else
      diff += bestCandidate->GetRadius();
    
  
    return diff;
  }
};

class EvalNearestYPerimeter : public EvalNearest
{
public:
  EvalNearestYPerimeter(WorldObject* o, double range):EvalNearest(o, range){}

  virtual double GetOutput()const
  {
    float diff = bestCandidateVec.y - owner->GetLocation().y;
    //cout << diff << endl;
    
    
    if (diff >= 0.0)
      diff -= bestCandidate->GetRadius();
    else
      diff += bestCandidate->GetRadius();
    
  
    return diff;
  }
};

template <class T>
Sensor* NearestNonScaleXSensor()
{
  Sensor* s = new Sensor(Vector2D(0.0, 0.0), 0.0);
  s->SetMatchingFunction(new MatchKindOf<T>);
  s->SetEvaluationFunction(new EvalNearestXPerimeter(s, 1000.0));
  s->SetScalingFunction(new ScaleAbs());
  
  return s;
}

template <class T>
Sensor* NearestNonScaleYSensor()
{
  Sensor* s = new Sensor(Vector2D(0.0, 0.0), 0.0);
  s->SetMatchingFunction(new MatchKindOf<T>);
  s->SetEvaluationFunction(new EvalNearestYPerimeter(s, 1000.0));
  s->SetScalingFunction(new ScaleAbs());
  return s;
}

template <class T>
Sensor* ColourSensor(double range, double alpha, int colour)
{
  Sensor* s = new AreaSensor(Vector2D(0.0, 0.0), 0.0);
  //Sensor* s = new BeamSensor(TWOPI, range, Vector2D(0.0, 0.0), 0);
  s->SetRadius(range);
  s->SetMatchingFunction(new MatchKindOf<T>);
  s->SetEvaluationFunction(new EvalColour(colour));
  s->SetScalingFunction(new ScaleByAlpha(alpha));
  return s;
}

template <class T>
Sensor* CountSensor(double range)
{
  Sensor* s = new AreaSensor(Vector2D(0.0, 0.0), 0.0);
  //Sensor* s = new BeamSensor(TWOPI, range);
  s->SetRadius(range);
  s->SetMatchingFunction(new MatchKindOf<T>);
  s->SetEvaluationFunction(new EvalCount());
  s->SetScalingFunction(new NoScale());
  return s;
}

#endif

