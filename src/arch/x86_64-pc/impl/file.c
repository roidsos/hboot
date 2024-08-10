#include <core/libc/file.h>

EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfs;

EFI_FILE_PROTOCOL *root;

HB_STATUS file_init(){
    EFI_GUID LI_GUID  = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID SFS_GUID = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

    EFI_LOADED_IMAGE_PROTOCOL *li;

    EFI_STATUS s = ST->BootServices->HandleProtocol(IH, &LI_GUID, (void**)&li);

    if (EFI_ERROR(s))
        return HB_FAIL;

    s = ST->BootServices->HandleProtocol(li->DeviceHandle, &SFS_GUID, (void**)&sfs);

    if (EFI_ERROR(s))
        return HB_FAIL;

    s = sfs->OpenVolume(sfs, &root);

    if (EFI_ERROR(s))
        return HB_FAIL;

    return HB_SUCCESS;
}

HB_FILE* file_open(CHAR16 *path, int mode){
    
    EFI_FILE_PROTOCOL *file = NULL;
    EFI_STATUS s = root->Open(root, &file, path, mode, 0);
    return EFI_ERROR(s) ? NULL : file;
}

HB_STATUS file_close(HB_FILE* file)
{
    EFI_STATUS s = file->Close(file);

    if (s != EFI_SUCCESS)
        return HB_FAIL;
    
    return HB_SUCCESS;
}

HB_STATUS file_read(HB_FILE* file, void *buf, size_t size)
{
    EFI_STATUS s = file->Read(file, &size, buf);
    if (s != EFI_SUCCESS)
        return HB_FAIL;
    return HB_SUCCESS;
}

HB_STATUS file_write(HB_FILE* file, void *buf, size_t size)
{
    EFI_STATUS s = file->Write(file, &size, buf);
    if (s != EFI_SUCCESS)
        return HB_FAIL;
    return HB_SUCCESS;
}

HB_STATUS file_seek(HB_FILE* file, int offset)
{
    EFI_STATUS s = file->SetPosition(file, offset);
    if (s != EFI_SUCCESS)
        return HB_FAIL;
    return HB_SUCCESS;
}

HB_SIZE file_size(HB_FILE* file)
{
    EFI_GUID FileInfoGuid = EFI_FILE_INFO_GUID;

    EFI_FILE_INFO info;
    EFI_UINTN infosize = sizeof(info);
    file->GetInfo(file, &FileInfoGuid,&infosize, &info);
    return info.FileSize;
}