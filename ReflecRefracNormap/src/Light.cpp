#include "SFW.h"
#include "Light.h"

Light::Light()
{
  //start initial position
  //at 5 units above the origin
  Pos_.x = 0;
  Pos_.y = 5;
  Pos_.z = 0;
}

Light::~Light()
{
}

Light::Light(SFW::Vec3 Pos)
{
  Pos_.x = Pos.x;
  Pos_.y = Pos.y;
  Pos_.z = Pos.z;
}