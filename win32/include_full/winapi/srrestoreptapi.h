/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the w64 mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within this package.
 */
#if !defined(_SRRESTOREPTAPI_H)
#define _SRRESTOREPTAPI_H

#define MIN_EVENT 100
#define BEGIN_SYSTEM_CHANGE 100
#define END_SYSTEM_CHANGE 101
#define BEGIN_NESTED_SYSTEM_CHANGE 102
#define END_NESTED_SYSTEM_CHANGE 103
#define MAX_EVENT 103

#define MIN_RPT 0
#define APPLICATION_INSTALL 0
#define APPLICATION_UNINSTALL 1
#define DESKTOP_SETTING 2
#define ACCESSIBILITY_SETTING 3
#define OE_SETTING 4
#define APPLICATION_RUN 5
#define RESTORE 6
#define CHECKPOINT 7
#define WINDOWS_SHUTDOWN 8
#define WINDOWS_BOOT 9
#define DEVICE_DRIVER_INSTALL 10
#define FIRSTRUN 11
#define MODIFY_SETTINGS 12
#define CANCELLED_OPERATION 13
#define BACKUP_RECOVERY 14
#define MAX_RPT 14

#define MAX_DESC 64
#define MAX_DESC_W 256

#pragma pack(push,srrestoreptapi_include)
#pragma pack(1)

typedef struct _RESTOREPTINFOA {
  DWORD dwEventType;
  DWORD dwRestorePtType;
  INT64 llSequenceNumber;
  CHAR szDescription[MAX_DESC];
} RESTOREPOINTINFOA,*PRESTOREPOINTINFOA;

typedef struct _RESTOREPTINFOW {
  DWORD dwEventType;
  DWORD dwRestorePtType;
  INT64 llSequenceNumber;
  WCHAR szDescription[MAX_DESC_W];
} RESTOREPOINTINFOW,*PRESTOREPOINTINFOW;

typedef struct _SMGRSTATUS {
  DWORD nStatus;
  INT64 llSequenceNumber;
} STATEMGRSTATUS,*PSTATEMGRSTATUS;

#pragma pack(pop,srrestoreptapi_include)

#ifdef __cplusplus
extern "C" {
#endif

  WINBOOL __stdcall SRSetRestorePointA(PRESTOREPOINTINFOA pRestorePtSpec,PSTATEMGRSTATUS pSMgrStatus);
  WINBOOL __stdcall SRSetRestorePointW(PRESTOREPOINTINFOW pRestorePtSpec,PSTATEMGRSTATUS pSMgrStatus);
  DWORD __stdcall SRRemoveRestorePoint(DWORD dwRPNum);

#ifdef __cplusplus
}
#endif

#ifdef UNICODE
#define RESTOREPOINTINFO RESTOREPOINTINFOW
#define PRESTOREPOINTINFO PRESTOREPOINTINFOW
#define SRSetRestorePoint SRSetRestorePointW
#else
#define RESTOREPOINTINFO RESTOREPOINTINFOA
#define PRESTOREPOINTINFO PRESTOREPOINTINFOA
#define SRSetRestorePoint SRSetRestorePointA
#endif
#endif
