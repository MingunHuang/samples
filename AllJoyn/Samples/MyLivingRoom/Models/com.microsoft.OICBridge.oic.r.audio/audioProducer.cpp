//-----------------------------------------------------------------------------
// <auto-generated> 
//   This code was generated by a tool. 
// 
//   Changes to this file may cause incorrect behavior and will be lost if  
//   the code is regenerated.
//
//   Tool: AllJoynCodeGenerator.exe
//
//   This tool is located in the Windows 10 SDK and the Windows 10 AllJoyn 
//   Visual Studio Extension in the Visual Studio Gallery.  
//
//   The generated code should be packaged in a Windows 10 C++/CX Runtime  
//   Component which can be consumed in any UWP-supported language using 
//   APIs that are available in Windows.Devices.AllJoyn.
//
//   Using AllJoynCodeGenerator - Invoke the following command with a valid 
//   Introspection XML file and a writable output directory:
//     AllJoynCodeGenerator -i <INPUT XML FILE> -o <OUTPUT DIRECTORY>
// </auto-generated>
//-----------------------------------------------------------------------------
#include "pch.h"

using namespace concurrency;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Devices::AllJoyn;
using namespace com::microsoft::OICBridge::oic::r::audio;

std::map<alljoyn_busobject, WeakReference*> audioProducer::SourceObjects;
std::map<alljoyn_interfacedescription, WeakReference*> audioProducer::SourceInterfaces;

audioProducer::audioProducer(AllJoynBusAttachment^ busAttachment)
    : m_busAttachment(busAttachment),
    m_sessionListener(nullptr),
    m_busObject(nullptr),
    m_sessionPort(0),
    m_sessionId(0)
{
    m_weak = new WeakReference(this);
    ServiceObjectPath = ref new String(L"/Service");
    m_signals = ref new audioSignals();
    m_busAttachmentStateChangedToken.Value = 0;
}

audioProducer::~audioProducer()
{
    UnregisterFromBus();
    delete m_weak;
}

void audioProducer::UnregisterFromBus()
{
    if ((nullptr != m_busAttachment) && (0 != m_busAttachmentStateChangedToken.Value))
    {
        m_busAttachment->StateChanged -= m_busAttachmentStateChangedToken;
        m_busAttachmentStateChangedToken.Value = 0;
    }
    if (nullptr != SessionPortListener)
    {
        alljoyn_busattachment_unbindsessionport(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), m_sessionPort);
        alljoyn_sessionportlistener_destroy(SessionPortListener);
        SessionPortListener = nullptr;
    }
    if (nullptr != BusObject)
    {
        alljoyn_busattachment_unregisterbusobject(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), BusObject);
        alljoyn_busobject_destroy(BusObject);
        BusObject = nullptr;
    }
    if (nullptr != SessionListener)
    {
        alljoyn_sessionlistener_destroy(SessionListener);
        SessionListener = nullptr;
    }
}

bool audioProducer::OnAcceptSessionJoiner(_In_ alljoyn_sessionport sessionPort, _In_ PCSTR joiner, _In_ const alljoyn_sessionopts opts)
{
    UNREFERENCED_PARAMETER(sessionPort); UNREFERENCED_PARAMETER(joiner); UNREFERENCED_PARAMETER(opts);
    
    return true;
}

void audioProducer::OnSessionJoined(_In_ alljoyn_sessionport sessionPort, _In_ alljoyn_sessionid id, _In_ PCSTR joiner)
{
    UNREFERENCED_PARAMETER(joiner);

    // We initialize the Signals object after the session has been joined, because it needs
    // the session id.
    m_signals->Initialize(BusObject, id);
    m_sessionPort = sessionPort;
    m_sessionId = id;

    alljoyn_sessionlistener_callbacks callbacks =
    {
        AllJoynHelpers::SessionLostHandler<audioProducer>,
        AllJoynHelpers::SessionMemberAddedHandler<audioProducer>,
        AllJoynHelpers::SessionMemberRemovedHandler<audioProducer>
    };

    SessionListener = alljoyn_sessionlistener_create(&callbacks, m_weak);
    alljoyn_busattachment_setsessionlistener(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), id, SessionListener);
}

void audioProducer::OnSessionLost(_In_ alljoyn_sessionid sessionId, _In_ alljoyn_sessionlostreason reason)
{
    if (sessionId == m_sessionId)
    {
        AllJoynSessionLostEventArgs^ args = ref new AllJoynSessionLostEventArgs(static_cast<AllJoynSessionLostReason>(reason));
        AllJoynHelpers::DispatchEvent([=]() {
            SessionLost(this, args);
        });
    }
}

void audioProducer::OnSessionMemberAdded(_In_ alljoyn_sessionid sessionId, _In_ PCSTR uniqueName)
{
    if (sessionId == m_sessionId)
    {
        auto args = ref new AllJoynSessionMemberAddedEventArgs(AllJoynHelpers::MultibyteToPlatformString(uniqueName));
        AllJoynHelpers::DispatchEvent([=]() {
            SessionMemberAdded(this, args);
        });
    }
}

void audioProducer::OnSessionMemberRemoved(_In_ alljoyn_sessionid sessionId, _In_ PCSTR uniqueName)
{
    if (sessionId == m_sessionId)
    {
        auto args = ref new AllJoynSessionMemberRemovedEventArgs(AllJoynHelpers::MultibyteToPlatformString(uniqueName));
        AllJoynHelpers::DispatchEvent([=]() {
            SessionMemberRemoved(this, args);
        });
    }
}

void audioProducer::BusAttachmentStateChanged(_In_ AllJoynBusAttachment^ sender, _In_ AllJoynBusAttachmentStateChangedEventArgs^ args)
{
    if (args->State == AllJoynBusAttachmentState::Connected)
    {   
        QStatus result = AllJoynHelpers::CreateProducerSession<audioProducer>(m_busAttachment, m_weak);
        if (ER_OK != result)
        {
            StopInternal(result);
            return;
        }
    }
    else if (args->State == AllJoynBusAttachmentState::Disconnected)
    {
        StopInternal(ER_BUS_STOPPING);
    }
}

QStatus audioProducer::AddMethodHandler(_In_ alljoyn_interfacedescription interfaceDescription, _In_ PCSTR methodName, _In_ alljoyn_messagereceiver_methodhandler_ptr handler)
{
    alljoyn_interfacedescription_member member;
    if (!alljoyn_interfacedescription_getmember(interfaceDescription, methodName, &member))
    {
        return ER_BUS_INTERFACE_NO_SUCH_MEMBER;
    }

    return alljoyn_busobject_addmethodhandler(
        m_busObject,
        member,
        handler,
        m_weak);
}

QStatus audioProducer::AddSignalHandler(_In_ alljoyn_busattachment busAttachment, _In_ alljoyn_interfacedescription interfaceDescription, _In_ PCSTR methodName, _In_ alljoyn_messagereceiver_signalhandler_ptr handler)
{
    alljoyn_interfacedescription_member member;
    if (!alljoyn_interfacedescription_getmember(interfaceDescription, methodName, &member))
    {
        return ER_BUS_INTERFACE_NO_SUCH_MEMBER;
    }

    return alljoyn_busattachment_registersignalhandler(busAttachment, handler, member, NULL);
}

QStatus audioProducer::OnPropertyGet(_In_ PCSTR interfaceName, _In_ PCSTR propertyName, _Inout_ alljoyn_msgarg value)
{
    UNREFERENCED_PARAMETER(interfaceName);

    if (0 == strcmp(propertyName, "Mute"))
    {
        auto task = create_task(Service->GetMuteAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "b", result->Mute));
    }
    if (0 == strcmp(propertyName, "Volume"))
    {
        auto task = create_task(Service->GetVolumeAsync(nullptr));
        auto result = task.get();
        
        if (AllJoynStatus::Ok != result->Status)
        {
            return static_cast<QStatus>(result->Status);
        }

        return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "x", result->Volume));
    }

    return ER_BUS_NO_SUCH_PROPERTY;
}

QStatus audioProducer::OnPropertySet(_In_ PCSTR interfaceName, _In_ PCSTR propertyName, _In_ alljoyn_msgarg value)
{
    UNREFERENCED_PARAMETER(interfaceName);

    if (0 == strcmp(propertyName, "Mute"))
    {
        bool argument;
        TypeConversionHelpers::GetAllJoynMessageArg(value, "b", &argument);

        auto task = create_task(Service->SetMuteAsync(nullptr, argument));
        auto result = task.get();

        return static_cast<QStatus>(result->Status);
    }
    if (0 == strcmp(propertyName, "Volume"))
    {
        int64 argument;
        TypeConversionHelpers::GetAllJoynMessageArg(value, "x", &argument);

        auto task = create_task(Service->SetVolumeAsync(nullptr, argument));
        auto result = task.get();

        return static_cast<QStatus>(result->Status);
    }
    return ER_BUS_NO_SUCH_PROPERTY;
}

void audioProducer::EmitMuteChanged()
{
    create_task([&]
	{
        alljoyn_msgarg value = alljoyn_msgarg_create();
        OnPropertyGet("com.microsoft.OICBridge.oic.r.audio", "Mute", value);

        alljoyn_busobject_emitpropertychanged(
            m_busObject,
            "com.microsoft.OICBridge.oic.r.audio",
            "Mute",
            value,
            m_sessionId);

        alljoyn_msgarg_destroy(value);
    });
}

void audioProducer::EmitVolumeChanged()
{
    create_task([&]
	{
        alljoyn_msgarg value = alljoyn_msgarg_create();
        OnPropertyGet("com.microsoft.OICBridge.oic.r.audio", "Volume", value);

        alljoyn_busobject_emitpropertychanged(
            m_busObject,
            "com.microsoft.OICBridge.oic.r.audio",
            "Volume",
            value,
            m_sessionId);

        alljoyn_msgarg_destroy(value);
    });
}

void audioProducer::Start()
{
    if (nullptr == m_busAttachment)
    {
        StopInternal(ER_FAIL);
        return;
    }

    QStatus result = AllJoynHelpers::CreateInterfaces(m_busAttachment, c_audioIntrospectionXml);
    if (result != ER_OK)
    {
        StopInternal(result);
        return;
    }

    result = AllJoynHelpers::CreateBusObject<audioProducer>(m_weak);
    if (result != ER_OK)
    {
        StopInternal(result);
        return;
    }

    alljoyn_interfacedescription interfaceDescription = alljoyn_busattachment_getinterface(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), "com.microsoft.OICBridge.oic.r.audio");
    if (interfaceDescription == nullptr)
    {
        StopInternal(ER_FAIL);
        return;
    }
    alljoyn_busobject_addinterface_announced(BusObject, interfaceDescription);

    
    SourceObjects[m_busObject] = m_weak;
    SourceInterfaces[interfaceDescription] = m_weak;
    
    unsigned int noneMechanismIndex = 0;
    bool authenticationMechanismsContainsNone = m_busAttachment->AuthenticationMechanisms->IndexOf(AllJoynAuthenticationMechanism::None, &noneMechanismIndex);
    QCC_BOOL interfaceIsSecure = alljoyn_interfacedescription_issecure(interfaceDescription);

    // If the current set of AuthenticationMechanisms supports authentication,
    // determine whether a secure BusObject is required.
    if (AllJoynHelpers::CanSecure(m_busAttachment->AuthenticationMechanisms))
    {
        // Register the BusObject as "secure" if the org.alljoyn.Bus.Secure XML annotation
        // is specified, or if None is not present in AuthenticationMechanisms.
        if (!authenticationMechanismsContainsNone || interfaceIsSecure)
        {
            result = alljoyn_busattachment_registerbusobject_secure(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), BusObject);
        }
        else
        {
            result = alljoyn_busattachment_registerbusobject(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), BusObject);
        }
    }
    else
    {
        // If the current set of AuthenticationMechanisms does not support authentication
        // but the interface requires security, report an error.
        if (interfaceIsSecure)
        {
            result = ER_BUS_NO_AUTHENTICATION_MECHANISM;
        }
        else
        {
            result = alljoyn_busattachment_registerbusobject(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), BusObject);
        }
    }

    if (result != ER_OK)
    {
        StopInternal(result);
        return;
    }

    m_busAttachmentStateChangedToken = m_busAttachment->StateChanged += ref new TypedEventHandler<AllJoynBusAttachment^,AllJoynBusAttachmentStateChangedEventArgs^>(this, &audioProducer::BusAttachmentStateChanged);
    m_busAttachment->Connect();
}

void audioProducer::Stop()
{
    StopInternal(AllJoynStatus::Ok);
}

void audioProducer::StopInternal(int32 status)
{
    UnregisterFromBus();
    Stopped(this, ref new AllJoynProducerStoppedEventArgs(status));
}

int32 audioProducer::RemoveMemberFromSession(_In_ String^ uniqueName)
{
    return alljoyn_busattachment_removesessionmember(
        AllJoynHelpers::GetInternalBusAttachment(m_busAttachment),
        m_sessionId,
        AllJoynHelpers::PlatformToMultibyteString(uniqueName).data());
}

PCSTR com::microsoft::OICBridge::oic::r::audio::c_audioIntrospectionXml = "<interface name=\"com.microsoft.OICBridge.oic.r.audio\">"
"  <property name=\"Mute\" type=\"b\" access=\"readwrite\">"
"    <annotation name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\" value=\"true\" />"
"  </property>"
"  <property name=\"Volume\" type=\"x\" access=\"readwrite\">"
"    <annotation name=\"org.freedesktop.DBus.Property.EmitsChangedSignal\" value=\"true\" />"
"  </property>"
"</interface>"
;
