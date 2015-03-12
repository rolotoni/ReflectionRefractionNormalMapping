/////////////////////////////////////
///definition of a light////////////
////////////////////////////////////

#ifndef LIGHT_H
#define LIGHT_H

class Light
{
  public:
    Light();
    ~Light();
    Light(SFW::Vec3);//add more to this constructor later
    //light position
  SFW::Vec3 Pos_;
  
    //to add
    //light type i.e. spot,directional etc.
    //light shape

};

#endif