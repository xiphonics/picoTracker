   Uint16 i_FileList = 0;
    int i, j;

    for(i=0; i<PSPSEQ_MAXFILES; i++)
    {
        for(j=0; j<18; j++)
        {
            PSPFileList[i][j] = 0;
        }
    }
    
    SceIoDirent de;
    
    memset(&de,0,sizeof(dirent));

    int fd, v;
    int idx=0;

    if(savetype == SAVETYPE_WAV)
    {
        fd=sceIoDopen(PSPWAVPath);
    }
    else
    {
        fd=sceIoDopen(PSPSEQPath);
    }
    if(fd<0)
    {
        NumFilesInList = 0;
        return;
    }
   
    v=sceIoDread(fd,&de);

    while((v!=0) && (idx<PSPSEQ_MAXFILES))
    {
        len=strlen(de.d_name);
        if(!strncmp(PSPFilePath,&de.d_name[len-4],4))
        {
            //len=strlen(de.d_name);
            if(len < 17)
            {
                strcpy(&PSPFileList[i_FileList++][0], de.d_name);
                idx++;
            }
        }
        v=sceIoDread(fd,&de);
    }
    NumFilesInList = idx;

    qsort((void*)PSPFileList, (size_t)NumFilesInList, 18, mycompare);

    if(savetype == SAVETYPE_WAV)
    {
        sceIoDclose(fd);
    }
    else
    {
        sceIoDclose(fd);
    }

