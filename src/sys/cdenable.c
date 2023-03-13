/*
 * HFVExplorer
 * Copyright (C) 1997-1999 by Anygraaf Oy
 * Author: Lauri Pesonen, email: lpesonen@clinet.fi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "ntddk.h"
#include "stdarg.h"
#include "stdio.h"
#include "cdenable.h"

#define DWORD unsigned int
#define WORD unsigned short
#define BYTE unsigned char

#ifdef _MYDEBUG
#define cdenableKdPrint(arg) DbgPrint arg
#else
#define cdenableKdPrint(arg)
#endif


// local forwards
NTSTATUS cdenableDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
VOID cdenableUnload(IN PDRIVER_OBJECT DriverObject);


NTSTATUS DriverEntry(
  IN PDRIVER_OBJECT  DriverObject,
  IN PUNICODE_STRING RegistryPath
) {
  PDEVICE_OBJECT         deviceObject        = NULL;
  NTSTATUS               ntStatus;
  WCHAR                  deviceNameBuffer[]  = L"\\Device\\cdenable";
  UNICODE_STRING         deviceNameUnicodeString;
  PDEVICE_EXTENSION      deviceExtension;
  WCHAR                  deviceLinkBuffer[]  = L"\\DosDevices\\cdenable";
  UNICODE_STRING         deviceLinkUnicodeString;

  cdenableKdPrint (("CDENABLE: entering DriverEntry\n"));

  RtlInitUnicodeString (&deviceNameUnicodeString,deviceNameBuffer);
  ntStatus = IoCreateDevice (DriverObject,
                             sizeof (DEVICE_EXTENSION),
                             &deviceNameUnicodeString,
                             FILE_DEVICE_CDENABLE,
                             0,
                             TRUE,
                             &deviceObject
                             );

  if (NT_SUCCESS(ntStatus)) {
    deviceExtension = (PDEVICE_EXTENSION) deviceObject->DeviceExtension;
    RtlInitUnicodeString (&deviceLinkUnicodeString,deviceLinkBuffer);
    ntStatus = IoCreateSymbolicLink (&deviceLinkUnicodeString,
                                     &deviceNameUnicodeString
                                     );
    if (!NT_SUCCESS(ntStatus)) {
      cdenableKdPrint (("CDENABLE: IoCreateSymbolicLink failed\n"));
    }
    DriverObject->MajorFunction[IRP_MJ_CREATE]         =
    DriverObject->MajorFunction[IRP_MJ_CLOSE]          =
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = cdenableDispatch;
    DriverObject->DriverUnload                         = cdenableUnload;
  } else {
    cdenableKdPrint(("CDENABLE: IoCreateDevice Failed\n"));
  }

  if (NT_SUCCESS(ntStatus)) {
    cdenableKdPrint(("Succes\n"));
  } else {
    cdenableKdPrint(("Failed, deleting device\n"));
    if (deviceObject) IoDeleteDevice (deviceObject);
  }
  return ntStatus;
}

static NTSTATUS CDenableIoCompletion(
  PDEVICE_OBJECT DeviceObject,
  PIRP Irp,
  PVOID Context)
{
  *Irp->UserIosb = Irp->IoStatus;
  KeSetEvent(Irp->UserEvent, 0, FALSE);
  IoFreeIrp(Irp);
  return( STATUS_MORE_PROCESSING_REQUIRED );
}

DWORD direct_read( HANDLE h, DWORD start, DWORD count, char *buf )
{
  DWORD result = 0;
  NTSTATUS status;
  PFILE_OBJECT FileObject;
  PMDL mdl;
  PIRP irp;
  KEVENT event;
  PIO_STACK_LOCATION ioStackLocation;
  // PDEVICE_OBJECT fsdDevice;
  PVOID buffer;
  IO_STATUS_BLOCK IoStatusBlock;
  LARGE_INTEGER currentOffset;
  PDEVICE_OBJECT targetDevice;

  cdenableKdPrint (("CDENABLE: direct_read\n"));

  currentOffset.QuadPart = 0;
  currentOffset.LowPart = start;
  IoStatusBlock.Status = STATUS_SUCCESS;

  status = ObReferenceObjectByHandle(
    h, FILE_READ_ACCESS, *IoFileObjectType, KernelMode, &FileObject, 0
  );
  if (NT_SUCCESS(status)) {
    // Now we must bypass the FSD and access directly the target device.
    // fsdDevice = IoGetRelatedDeviceObject(FileObject);
    // targetDevice = fsdDevice;


    // targetDevice = FileObject->DeviceObject;
    targetDevice = IoGetRelatedDeviceObject(FileObject);


    KeInitializeEvent(&event, SynchronizationEvent, FALSE);
    irp = IoAllocateIrp(targetDevice->StackSize, FALSE);
    if(irp) {
      buffer = ExAllocatePoolWithTag(
        NonPagedPool, CDENABLE_MAX_TRANSFER_SIZE, 'CDlp'
      );
      if (buffer) {
        mdl = IoAllocateMdl(buffer, CDENABLE_MAX_TRANSFER_SIZE, FALSE, TRUE, 0);
        MmBuildMdlForNonPagedPool(mdl);
        irp->MdlAddress = mdl;
        irp->UserEvent = &event;
        irp->UserIosb = &IoStatusBlock;
        irp->Tail.Overlay.Thread = PsGetCurrentThread();
        irp->Tail.Overlay.OriginalFileObject= FileObject;
        irp->RequestorMode = KernelMode;
        irp->Flags = IRP_READ_OPERATION;
        ioStackLocation = IoGetNextIrpStackLocation(irp);
        ioStackLocation->MajorFunction = IRP_MJ_READ;
        ioStackLocation->MinorFunction = 0;
        ioStackLocation->DeviceObject = targetDevice;
        ioStackLocation->FileObject = FileObject;
        IoSetCompletionRoutine(irp, CDenableIoCompletion, 0, TRUE, TRUE, TRUE);
        ioStackLocation->Parameters.Read.Length = count;
        ioStackLocation->Parameters.Read.ByteOffset = currentOffset;
        (void) IoCallDriver(targetDevice, irp);
        KeWaitForSingleObject(&event, Executive, KernelMode, TRUE, 0);
        // Note that the irp is gone now.
        if (NT_SUCCESS(IoStatusBlock.Status)) {
          result = 1;
          RtlCopyMemory( buf, buffer, count );
        }
        IoFreeMdl(mdl);
        ExFreePool(buffer);
      }
    }
    ObDereferenceObject(FileObject);
  }

  cdenableKdPrint (("CDENABLE: direct_read end\n"));

  return(result);
}

NTSTATUS cdenableDispatch( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
{
  PIO_STACK_LOCATION  irpStack;
  PDEVICE_EXTENSION   deviceExtension;
  PVOID               ioBuffer;
  PVOID               outBuffer;
  PVOID               outBuffer2;
  ULONG               inputBufferLength;
  ULONG               outputBufferLength;
  ULONG               ioControlCode;
  NTSTATUS            ntStatus;

  Irp->IoStatus.Status      = STATUS_SUCCESS;
  Irp->IoStatus.Information = 0;
  irpStack = IoGetCurrentIrpStackLocation (Irp);
  deviceExtension = DeviceObject->DeviceExtension;

  ioBuffer           = Irp->AssociatedIrp.SystemBuffer;
  outBuffer          = Irp->AssociatedIrp.SystemBuffer;
  outBuffer2         = Irp->UserBuffer;

  inputBufferLength  = irpStack->Parameters.DeviceIoControl.InputBufferLength;
  outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

  switch (irpStack->MajorFunction) {
    case IRP_MJ_CREATE:
      cdenableKdPrint (("CDENABLE: IRP_MJ_CREATE\n"));
      break;
    case IRP_MJ_CLOSE:
      cdenableKdPrint (("CDENABLE: IRP_MJ_CLOSE\n"));
      break;
    case IRP_MJ_DEVICE_CONTROL:
      cdenableKdPrint (("CDENABLE: IRP_MJ_DEVICE_CONTROL\n"));
      ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
      switch (ioControlCode) {
        case IOCTL_CDENABLE_GET_VERSION:
          cdenableKdPrint (("CDENABLE: get version\n"));
          *((DWORD *)outBuffer) = CDENABLE_CURRENT_VERSION;
          *((DWORD *)outBuffer2) = *((DWORD *)outBuffer);
          Irp->IoStatus.Information = 4; // length of return data
          break;
        case IOCTL_CDENABLE_READ:
        {
          DWORD h, start, count, retval;
          char *buf;

#ifdef _MYDEBUG
          try {
            _asm int 3
          } except( EXCEPTION_EXECUTE_HANDLER ) {
            // We don't want blue screens if we forget
            // to enable i3 in debugger ... just ignore it.
          }
#endif

          cdenableKdPrint (("CDENABLE: start read\n"));
          h     = ((DWORD *)ioBuffer)[0];
          start = ((DWORD *)ioBuffer)[1];
          count = ((DWORD *)ioBuffer)[2];
          buf   = (char *)((DWORD *)ioBuffer)[3];

          if(h == 0 || start < 0 || count > CDENABLE_MAX_TRANSFER_SIZE || buf == 0) {
            cdenableKdPrint (("CDENABLE: invalid parameter\n"));
            Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
          } else {
            try {
              retval = direct_read( (HANDLE)h, start, count, buf );
            } except( EXCEPTION_EXECUTE_HANDLER ) {
              retval = 0;
              cdenableKdPrint (("CDENABLE: unhandled exception in direct_read\n"));
            }
            *((DWORD *)outBuffer) = retval;
            *((DWORD *)outBuffer2) = *((DWORD *)outBuffer);
            Irp->IoStatus.Information = 4; // length of return data
          }
          cdenableKdPrint (("CDENABLE: end read\n"));
          break;
        }
        default:
          Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
          cdenableKdPrint (("CDENABLE: unknown IRP_MJ_DEVICE_CONTROL\n"));
          break;
      }
      break;
  }

  ntStatus = Irp->IoStatus.Status;
  IoCompleteRequest( Irp, IO_NO_INCREMENT );
  // No more Irp here...
  return ntStatus;
}

VOID cdenableUnload( IN PDRIVER_OBJECT DriverObject )
{
  WCHAR                  deviceLinkBuffer[]  = L"\\DosDevices\\cdenable";
  UNICODE_STRING         deviceLinkUnicodeString;

  cdenableKdPrint (("CDENABLE: unloading\n"));
  RtlInitUnicodeString (&deviceLinkUnicodeString,deviceLinkBuffer);
  IoDeleteSymbolicLink (&deviceLinkUnicodeString);
  IoDeleteDevice (DriverObject->DeviceObject);
}
