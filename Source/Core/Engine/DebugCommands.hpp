﻿#pragma once
namespace Plasma
{
    DeclareEnum10(GeometryValue, None, Normal, Depth, Roughness,  Albedo,  Metallic, Specular, Emissive, AO, SpecularColor)
  
    namespace Events
    {
        DeclareEvent(ScriptInitialize);
        DeclareEvent(DebugViewMode);
        DeclareEvent(ViewportCameraSettings);
    } // namespace Events

    class DebugViewEvent : public Event
    {
    public:
        LightningDeclareType(DebugViewEvent, TypeCopyMode::ReferenceType);
        DebugViewEvent(GeometryValue::Enum mode): mMode(mode)  { }
        DebugViewEvent()  { }
        ~DebugViewEvent() { }
        GeometryValue::Enum mMode = GeometryValue::Enum::None;
    };
    
    class ViewportCameraEvent : public Event
    {
    public:
        LightningDeclareType(ViewportCameraEvent, TypeCopyMode::ReferenceType);
        ViewportCameraEvent(float speed): mSpeed(speed)  { }
        ViewportCameraEvent()  { }
        ~ViewportCameraEvent() { }
        
        float mSpeed = 1.0f;
    };
    

} // Namespace Plasma