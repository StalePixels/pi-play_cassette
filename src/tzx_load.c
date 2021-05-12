#include "tzx_load.h"
#include "tzx_file.h"
#include "tzx-pi.h"

uint8_t tzx_load(char *filename) {
    struct stat st; // file metadata

    if(!stat(filename,&st))
    {
        tzx_filesize = st.st_size;
    }
    else
    {
        return ERR_TZX_FILE_SIZE_ERROR;
    }
    
    tzx_filehandle = fopen(filename,"rb");
    if(tzx_filehandle==NULL)
    {
        return ERR_TZX_FILE_OPEN_ERROR;
    }
    
    tzx_data=malloc(tzx_filesize);
    if(tzx_data==NULL)
    {
        return ERR_TZX_FILE_NO_MEMORY;
    }
    
    // Load the header
    fread(tzx_data, 10, 1, tzx_filehandle); 
    // Terminate the headerstring
    tzx_data[7]=0;
    
    
    if (strcmp(tzx_data,TZXTapeHeaderID))
    {
        return ERR_TZX_NOT_TZX_FILE;
    }

    if (!strcmp(tzx_data,TAPTapeHeaderID) )
    {
        return ERR_TZX_IS_ACTUALLY_TAP;
    }
    
    // Version wangling
    tzx_version_major = tzx_data[8];
    tzx_version_minor = tzx_data[9];

    if (!tzx_data[8])
    {
        return ERR_TZX_BETAS_NOT_SUPPORTED;
    }
    
    if (tzx_data[8]>TZX_SUPPORTED_MAJOR_REV)
    {
         printf("\n-- Warning: Some blocks may not be recognised and used!\n");
    }
    
    if (tzx_data[8]==TZX_SUPPORTED_MAJOR_REV && tzx_data[9]>TZX_SUPPORTED_MINOR_REV) {
        printf("\n-- Warning: Some of the data might not be properly recognised!\n");
    }
    
    // Load the actual TZX data
    fread(tzx_data, tzx_filesize-10, 1, tzx_filehandle); 
    
    return 0;
}