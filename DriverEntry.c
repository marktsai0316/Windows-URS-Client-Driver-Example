//https://msdn.microsoft.com/en-us/library/windows/hardware/hh451097(v=vs.85).aspx

//Supporting Functional Power States
//https://msdn.microsoft.com/en-us/library/windows/hardware/hh451017(v=vs.85).aspx
EVT_WDFDEVICE_WDM_POST_PO_FX_REGISTER_DEVICE EvtDevicePostPoFxRegister;
EVT_WDFDEVICE_WDM_PRE_PO_FX_UNREGISTER_DEVICE EvtDevicePrePoFxUnregister;

//UrsDeviceInitialize
//https://msdn.microsoft.com/en-us/library/windows/hardware/mt628012(v=vs.85).aspx
EVT_URS_DEVICE_FILTER_RESOURCE_REQUIREMENTS EvtUrsFilterResourceRequirements;
EVT_URS_SET_ROLE EvtUrsSetRole;

//https://msdn.microsoft.com/en-us/library/windows/hardware/mt595921(v=vs.85).aspx
//The USB dual-role class extension invokes this callback to allow the client driver 
//to insert the resources from the resource-requirements-list object to resource lists 
//that will be used during the life time of each role.
EVT_URS_DEVICE_FILTER_RESOURCE_REQUIREMENTS EvtUrsFilterResourceRequirements;

_Function_class_(EVT_URS_DEVICE_FILTER_RESOURCE_REQUIREMENTS)
_IRQL_requires_same_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
EvtUrsFilterResourceRequirements (
    _In_ WDFDEVICE Device,
    _In_ WDFIORESREQLIST IoResourceRequirementsList,
    _In_ URSIORESLIST HostRoleResources,
    _In_ URSIORESLIST FunctionRoleResources
    )
{
    NTSTATUS status;
    WDFIORESLIST resList;
    ULONG resListCount;
    ULONG resCount;
    ULONG currentResourceIndex;
    PIO_RESOURCE_DESCRIPTOR descriptor;
    BOOLEAN assignToHost;
    BOOLEAN assignToFunction;
    BOOLEAN keepAssigned;

    TRY {

        status = STATUS_SUCCESS;

        //
        // Currently does not support multiple logical configurations. Only the first one
        // is considered.
        //

        resListCount = WdfIoResourceRequirementsListGetCount(IoResourceRequirementsList);
        if (resListCount == 0) {
            // No logical resource configurations found.
            LEAVE;
        }

        // Enumerate through logical resource configurations.

        resList = WdfIoResourceRequirementsListGetIoResList(IoResourceRequirementsList, 0);
        resCount = WdfIoResourceListGetCount(resList);


        for (currentResourceIndex = 0; currentResourceIndex < resCount; ++currentResourceIndex) {

            descriptor = WdfIoResourceListGetDescriptor(resList, currentResourceIndex);

            if (descriptor->Type == CmResourceTypeConfigData) {

                //
                // This indicates the priority of this logical configuration.
                // This descriptor can be ignored.
                //

                keepAssigned = TRUE;
                assignToFunction = FALSE;
                assignToHost = FALSE;

            } else if ((descriptor->Type == CmResourceTypeMemory) ||
                       (descriptor->Type == CmResourceTypeMemoryLarge)) {

                //
                // This example client driver keeps the memory resources here. 
                //

                keepAssigned = TRUE;
                assignToFunction = TRUE;
                assignToHost = TRUE;

            } else {

                //
                // For all other resources, pass it to the child device nodes for host and function.
                //

                keepAssigned = FALSE;
                assignToHost = TRUE;
                assignToFunction = TRUE;
            }

            if (assignToHost != FALSE) {
                status = UrsIoResourceListAppendDescriptor(HostRoleResources, descriptor);
                if (!NT_SUCCESS(status)) {
                    // UrsIoResourceListAppendDescriptor for HostRoleResources failed.
                    LEAVE;
                }
            }

            if (assignToFunction != FALSE) {
                status = UrsIoResourceListAppendDescriptor(FunctionRoleResources, descriptor);
                if (!NT_SUCCESS(status)) {
                    // UrsIoResourceListAppendDescriptor for FunctionRoleResources failed.
                    LEAVE;
                }
            }

            if (keepAssigned == FALSE) {
                WdfIoResourceListRemove(resList, currentResourceIndex);
                --currentResourceIndex;
                --resCount;
            }
        }

    } FINALLY {
    }

    return status;
}

//https://msdn.microsoft.com/en-us/library/windows/hardware/mt595922(v=vs.85).aspx
NTSTATUS
EvtUrsSetRole (
    _In_ WDFDEVICE Device,
    _In_ URS_ROLE Role
    )
{
    NTSTATUS status;
    PFDO_CONTEXT fdoContext;

    TRACE_FUNC_ENTRY(TRACE_FLAG);
    TRY {


           // Change the current role of the controller to the specified role.
           // The driver might have stored the control registers in the device context. 
           // Read and write the register to get and set the current role. 

        }


        TRACE_INFO(TRACE_FLAG, "[Device: 0x%p] Successfully set role to %!URS_ROLE!", Device, Role);

        status = STATUS_SUCCESS;

    } FINALLY {

    }

    TRACE_FUNC_EXIT(TRACE_FLAG);

    return status;
}
//https://msdn.microsoft.com/en-us/library/windows/hardware/ff540880(v=vs.85).aspx
//To register an EvtDevicePrepareHardware callback function, a driver must call WdfDeviceInitSetPnpPowerEventCallbacks.
EVT_WDF_DEVICE_PREPARE_HARDWARE EvtDevicePrepareHardware;

NTSTATUS EvtDevicePrepareHardware(
  _In_ WDFDEVICE    Device,
  _In_ WDFCMRESLIST ResourcesRaw,
  _In_ WDFCMRESLIST ResourcesTranslated
)
{ ... }



EvtDriverDeviceAdd (
    _In_ WDFDRIVER Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
{
...

    WDFDEVICE device;
    NTSTATUS status;
...
//Initializes device initialization operations when the Plug and Play (PnP) manager reports the existence of a device
    status = UrsDeviceInitInitialize(DeviceInit);
    if (!NT_SUCCESS(status)) {
        //UrsDeviceInitInitialize failed.
        return status;
    }
    
    
  
WDF_PNPPOWER_EVENT_CALLBACKS  pnpPowerCallbacks;
    //
    // Zero out the PnpPowerCallbacks structure.
    //
WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);
pnpPowerCallbacks.EvtDevicePrepareHardware = EvtDevicePrepareHardware;
//pnpPowerCallbacks.EvtDeviceReleaseHardware = EvtReleaseHardware;
//pnpPowerCallbacks.EvtDeviceD0Entry = DrvEvtDeviceD0Entry;
//pnpPowerCallbacks.EvtDeviceD0Exit = DrvEvtDeviceD0Exit;
//pnpPowerCallbacks.EvtDeviceD0EntryPostInterruptsEnabled = NICEvtDeviceD0EntryPostInterruptsEnabled;
//pnpPowerCallbacks.EvtDeviceD0ExitPreInterruptsDisabled = NICEvtDeviceD0ExitPreInterruptsDisabled;
//   pnpPowerCallbacks.EvtDeviceSelfManagedIoInit    = PciDrvEvtDeviceSelfManagedIoInit;
//   pnpPowerCallbacks.EvtDeviceSelfManagedIoCleanup = PciDrvEvtDeviceSelfManagedIoCleanup;
//   pnpPowerCallbacks.EvtDeviceSelfManagedIoSuspend = PciDrvEvtDeviceSelfManagedIoSuspend;
//   pnpPowerCallbacks.EvtDeviceSelfManagedIoRestart = PciDrvEvtDeviceSelfManagedIoRestart;

    //
    // Register the PnP and power callbacks. Power policy related callbacks will be registered
    // later.
    // 
WdfDeviceInitSetPnpPowerEventCallbacks(
                                       DeviceInit,
                                       &pnpPowerCallbacks
                                       );
WDF_POWER_POLICY_EVENT_CALLBACKS powerPolicyCallbacks;
    //
    // Init the power policy callbacks
    //  

WDF_POWER_POLICY_EVENT_CALLBACKS_INIT(&powerPolicyCallbacks);
    //
    // This group of three callbacks allows this sample driver to manage
    // arming the device for wake from the S0 state.  Networking devices can
    // optionally be put into a low-power state when there is no networking
    // cable plugged into them.  This sample implements this feature.
    //
powerPolicyCallbacks.EvtDeviceArmWakeFromS0 = PciDrvEvtDeviceWakeArmS0;
powerPolicyCallbacks.EvtDeviceDisarmWakeFromS0 = PciDrvEvtDeviceWakeDisarmS0;
powerPolicyCallbacks.EvtDeviceWakeFromS0Triggered = PciDrvEvtDeviceWakeTriggeredS0;
    //
    // This group of three callbacks allows the device to be armed for wake
    // from Sx (S1, S2, S3 or S4.)  Networking devices can optionally be put
    // into a state where a packet sent to them will cause the device's wake
    // signal to be triggered, which causes the machine to wake, moving back
    // into the S0 state.
    //
powerPolicyCallbacks.EvtDeviceArmWakeFromSx = PciDrvEvtDeviceWakeArmSx;
powerPolicyCallbacks.EvtDeviceDisarmWakeFromSx = PciDrvEvtDeviceWakeDisarmSx;
powerPolicyCallbacks.EvtDeviceWakeFromSxTriggered = PciDrvEvtDeviceWakeTriggeredSx;
    //
    // Register the power policy callbacks.
    //
WdfDeviceInitSetPowerPolicyEventCallbacks(
                                          DeviceInit,
                                          &powerPolicyCallbacks
                                          );
  
    // Since we are the function driver, we are now the power policy owner
    // for the device according to the default framework rule. We will register
    // our power policy callbacks after finding the wakeup capability of the device.
 
    //
    // Specify the context type and size for the device we are about to create.
    //  
   
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, DRIVER_CONTEXT); //FDO_DATA ????
    //
    // ContextCleanup will be called by the framework when it deletes the device.
    // So you can defer freeing any resources allocated to Cleanup callback in the
    // event AddDevice returns any error after the device is created.
    //
    //attributes.EvtCleanupCallback = PciDrvEvtDeviceContextCleanup;
    status = WdfDeviceCreate(&DeviceInit, &attributes, &device);
    if (!NT_SUCCESS(status)) {
        // WdfDeviceCreate failed.
        return status;
    }
    URS_CONFIG_INIT(&ursConfig, UrsHostInterfaceTypeXhci, EvtUrsFilterResourceRequirements);
    
    ursConfig.EvtUrsSetRole = EvtUrsSetRole;
    status = UrsDeviceInitialize(device, &ursConfig);

    if (!NT_SUCCESS(status)) {
        // UrsDeviceInitialize failed.
        return status;

    }

    WDF_DEVICE_POWER_POLICY_IDLE_SETTINGS idleSettings;
    WDF_POWER_FRAMEWORK_SETTINGS poFxSettings;
...
 
    TRY {
// registers power management framework (PoFx) settings for single-component devices.
        WDF_POWER_FRAMEWORK_SETTINGS_INIT(&poFxSettings);
        poFxSettings.EvtDeviceWdmPostPoFxRegisterDevice = EvtDevicePostPoFxRegister;
        poFxSettings.EvtDeviceWdmPrePoFxUnregisterDevice = EvtDevicePrePoFxUnregister;

        status = WdfDeviceWdmAssignPowerFrameworkSettings(device, &poFxSettings);
        if (!NT_SUCCESS(status)) {
            // WdfDeviceWdmAssignPowerFrameworkSettings failed.

        }

        WDF_DEVICE_POWER_POLICY_IDLE_SETTINGS_INIT(&idleSettings, IdleCannotWakeFromS0);
        idleSettings.IdleTimeoutType = SystemManagedIdleTimeout;

        status = WdfDeviceAssignS0IdleSettings(device, &idleSettings);
        if (!NT_SUCCESS(status)) {
            // WdfDeviceAssignS0IdleSettings failed.

        }

    } FINALLY {
    }

..

}

NTSTATUS
EvtDevicePostPoFxRegister (
    _In_ WDFDEVICE Device,
    _In_ POHANDLE PoHandle
    )
{
    UrsSetPoHandle(Device, PoHandle);

    return STATUS_SUCCESS;
}


VOID
EvtDevicePrePoFxUnregister (
    _In_ WDFDEVICE Device,
    _In_ POHANDLE PoHandle
    )
{
    UNREFERENCED_PARAMETER(PoHandle);

    UrsSetPoHandle(Device, NULL);
}

//http://www.getcodesamples.com/src/2DA5B5AE/9B6404C3
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
/*++
 
Routine Description:
 
    Installable driver initialization entry point.
    This entry point is called directly by the I/O system.
 
Arguments:
 
    DriverObject - pointer to the driver object
 
    RegistryPath - pointer to a unicode string representing the path,
                   to driver-specific key in the registry.
 
Return Value:
 
    STATUS_SUCCESS if successful,
    STATUS_UNSUCCESSFUL otherwise.
 
--*/
{
    NTSTATUS               status = STATUS_SUCCESS;
    WDF_DRIVER_CONFIG      config;
    WDF_OBJECT_ATTRIBUTES  attrib;
    WDFDRIVER              driver;
    PDRIVER_CONTEXT        driverContext;
 
    //
    // Initialize WPP Tracing
    //
    WPP_INIT_TRACING( DriverObject, RegistryPath );
 
    TraceEvents(TRACE_LEVEL_INFORMATION, DBG_INIT, "Sample - Driver Framework Edition \n");
 
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attrib, DRIVER_CONTEXT);
 
    //
    // Register a cleanup callback so that we can call WPP_CLEANUP when
    // the framework driver object is deleted during driver unload.
    //
    attrib.EvtCleanupCallback = PciDrvEvtDriverContextCleanup;
 
    //
    // Initialize the Driver Config structure..
    //
    WDF_DRIVER_CONFIG_INIT(&config, PciDrvEvtDeviceAdd);
 
    //
    // Create a WDFDRIVER object.
    //
    status = WdfDriverCreate(DriverObject,
                             RegistryPath,
                             &attrib,
                             &config,
                             &driver);
 
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, DBG_INIT,
                    "WdfDriverCreate failed with status %!STATUS!\n", status);
        //
        // Cleanup tracing here because DriverContextCleanup will not be called
        // as we have failed to create WDFDRIVER object itself.
        // Please note that if your return failure from DriverEntry after the
        // WDFDRIVER object is created successfully, you don't have to
        // call WPP cleanup because in those cases DriverContextCleanup
        // will be executed when the framework deletes the DriverObject.
        //
        WPP_CLEANUP(DriverObject);
        return status;
    }
 
    driverContext = GetDriverContext(driver);
 
    //
    // Create a driver wide lookside list used for allocating memory  for the
    // MP_RFD structure for all device instances (if there are multiple present).
    //
    status = WdfLookasideListCreate(WDF_NO_OBJECT_ATTRIBUTES, // LookAsideAttributes
                                sizeof(MP_RFD),
                                NonPagedPool,
                                WDF_NO_OBJECT_ATTRIBUTES, // MemoryAttributes
                                PCIDRV_POOL_TAG,
                                &driverContext->RecvLookaside
                                );
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, DBG_INIT,
                    "Couldn't allocate lookaside list status %!STATUS!\n", status);
        return status;
    }
 
    return status;
 
}
