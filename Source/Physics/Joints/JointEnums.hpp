#pragma once

namespace Plasma
{

namespace JointEnums
{

//Define an enum that constraints all of the joint types and the total joint count.
#define JointType(type) type##Type,

  enum JointTypes
  {
#include "Physics/Joints/JointList.hpp"
      JointCount
  };

#undef JointType

}//namespace JointEnums

// Additionally define JointTypes of the same enum specification zero needs for binding
namespace JointTypes
{

//Define an enum that constraints all of the joint types and the total joint count.
#define JointType(type) type,

// Declare the actual Enum
enum Enum
{
#include "Physics/Joints/JointList.hpp"
  Size
};
#undef JointType

// Define the Names for binding
#define JointType(type) #type,
static const cstr Names[] =
{
#include "Physics/Joints/JointList.hpp"
  "Size"
};
#undef JointType

}//namespace JointTypes

}//namespace Plasma
