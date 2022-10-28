#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<iostream>
//#include<io.h>

#define MAXINODE 50

#define READ 1
#define WRITE 2

#define MAXFLESIZE 1024

#define REGULAR 1
#define SPECIAL 2

#define START 0 
#define CURRENT 1
#define END 2

typedef struct superblock
{
    int TotalInodes;
    int FreeInode;
}SUPERBLOCK, *PSUPERBLOCK;

typedef struct inode
{
    char FileName[50];
    int InodeNumber;
    int FileSize;
    int FileActualSize;
    int FileType;
    char *Buffer;
    int LinkCount;
    int ReferenceCount;
    int permission;
    struct inode *next;
}INODE, *PINODE, **PPINODE;

typedef struct filetable
{
    int readoffset;
    int writeoffset;
    int count;
    int mode;
    PINODE ptrinode;
}FILETABLE, *PFILETABLE;

typedef  struct ufdt
{
    PFILETABLE ptrfiletable;
}UFDT;

UFDT UFDTArr[MAXINODE];
SUPERBLOCK SUPERBLOCKobj;
PINODE head = NULL;

void man(char *name)
{
    if(name == NULL) return;

    if(strcmp(name,"create") == 0)
    {
        printf("Descprition : Used to create new regular file\n");
        printf("Syntax : create File_name Permission\n");
    }
    else if(strcmp(name,"read") == 0)
    {
        printf("Descprition : Used to read data from regular file\n");
        printf("Syntax : read File_name No_Of_Bytes_To_Read \n");
    }
    else if(strcmp(name,"write") == 0)
    {
        printf("Description : Used to write into regular file \n");
        printf("Syntax: write File_name\n After this enter the data that we want to write \n");
    }
    else if(strcmp(name,"ls") == 0)
    {
        printf("Description : Used to list all information of files \n");
        printf("Syntax: ls \n");
    }
    else if(strcmp(name,"stat") == 0)
    {
        printf("Description : Used to display information of file\n");
        printf("Syntax: stat File_name \n");
    }
    else if(strcmp(name,"open") == 0)
    {
        printf("Description : Used to open existing file \n");
        printf("Syntax: open File_name mode \n");
    }
    else if(strcmp(name,"close") == 0)
    {
        printf("Description : Used to close opened file\n");
        printf("Syntax: close File_name \n");
    }
    else if(strcmp(name,"rm") == 0)
    {
        printf("Description : Used to delete the file\n");
        printf("Syntax: rm File_name \n");
    }
    else
    {
        printf("ERROR : No manual entry available.");
    }
}

void DisplayHelp()
{
    printf("ls : To List out all files \n");
    printf("clear : To clear console \n");
    printf("open : To open the file \n");
    printf("close : To close the file\n");
    printf("read : To Read the contents from the file\n");
    printf("write : To write contents into the file\n");
    printf("exit : To terminate file system\n");
    printf("stat : To Display information of file using name\n");
    printf("rm : To Delete the file\n");
}

int GetFDFromName(char *name)
{
    int i = 0;

    while(i < MAXINODE)
    {
        if((UFDTArr[i].ptrfiletable != NULL) && (UFDTArr[i].ptrfiletable->ptrinode->FileType != 0))
            if(strcmp((UFDTArr[i].ptrfiletable->ptrinode->FileName),name) == 0)
                break;
        i++;
    }
    
    if(i == MAXINODE)     return -1;
    else            return i;
}

PINODE Get_INODE(char *name)
{
    PINODE temp = head;
    int i = 0;

    if(name == NULL)
        return NULL;

    while(temp != NULL)
    {
        if(strcmp(name, temp->FileName) == 0)
            break;
        temp = temp->next;
    }
    return temp;
}

void CreateDILB()
{
    int i = 1;
    PINODE newn = NULL;
    PINODE temp = head;

    while(i <= MAXINODE)
    {
        newn = (PINODE)malloc(sizeof(INODE));

        newn->LinkCount = 0;
        newn->ReferenceCount = 0;
        newn->FileType = 0;
        newn->FileSize = 0;

            newn->Buffer = NULL;
            newn->next = NULL;

            newn->InodeNumber = i;

            if(temp == NULL)
            {
                head = newn;
                temp = head;
            }
            else
            {
                temp->next = newn;
                temp = temp->next;
            }
            i++;
    }
}

void InitialiseSuperBlock()
{
    int i = 0;
    while(i < MAXINODE)
    {
        UFDTArr[i].ptrfiletable = NULL;
        i++;
    }

    SUPERBLOCKobj.TotalInodes = MAXINODE;
    SUPERBLOCKobj.FreeInode = MAXINODE;
}

int CreateFile(char *name, int permission)
{
    int i= 0;
    PINODE temp = head;

    if((name == NULL) || (permission == 0)  || (permission > 3))
        return -1;

    if(SUPERBLOCKobj.FreeInode == 0)
        return -2;

        (SUPERBLOCKobj.FreeInode)--;

    if(Get_INODE(name) != NULL)
        return -3;

    while(temp != NULL)
    {
        if(temp->FileType == 0)
            break;
        temp=temp->next;
    }

    while(i < MAXINODE)
    {
        if(UFDTArr[i].ptrfiletable == NULL)
            break;
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));

    UFDTArr[i].ptrfiletable->count = 1;
    UFDTArr[i].ptrfiletable->mode = permission;
    UFDTArr[i].ptrfiletable->readoffset = 0;
    UFDTArr[i].ptrfiletable->writeoffset = 0;

    UFDTArr[i].ptrfiletable->ptrinode = temp;

    strcpy(UFDTArr[i].ptrfiletable->ptrinode->FileName,name);
    UFDTArr[i].ptrfiletable->ptrinode->FileType = REGULAR;
    UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount = 1;
    UFDTArr[i].ptrfiletable->ptrinode->LinkCount = 1;
    UFDTArr[i].ptrfiletable->ptrinode->FileSize = MAXFLESIZE;
    UFDTArr[i].ptrfiletable->ptrinode->FileActualSize = 0;
    UFDTArr[i].ptrfiletable->ptrinode->permission = permission;
    UFDTArr[i].ptrfiletable->ptrinode->Buffer = (char*)malloc(MAXFLESIZE);

    return i;
}

int rm_File(char * name)
{
    int fd = 0;

    fd = GetFDFromName(name);
    if(fd == -1)
        return -1;

    (UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)--;

    if(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount == 0)
    {
        UFDTArr[fd].ptrfiletable->ptrinode->FileType = 0;
        free(UFDTArr[fd].ptrfiletable);
    }

    UFDTArr[fd].ptrfiletable = NULL;
    (SUPERBLOCKobj.FreeInode)++;

    return 0;
}

int ReadFile(int fd, char *arr, int iSize)
{
    int read_size = 0;

    if(UFDTArr[fd].ptrfiletable == NULL)    
    return -1;

    if(UFDTArr[fd].ptrfiletable->mode != READ && UFDTArr[fd].ptrfiletable->mode != READ+WRITE)  
    return -2;

    if(UFDTArr[fd].ptrfiletable->ptrinode->permission != READ && UFDTArr[fd].ptrfiletable->ptrinode->permission != READ+WRITE)  
    return -2;

    if(UFDTArr[fd].ptrfiletable->readoffset == UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)  
    return -3;

    if(UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR) 
    return -4;

    read_size = ( UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) - ( UFDTArr[fd].ptrfiletable->readoffset);
    
    if(read_size < iSize)
    {
        strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + ( UFDTArr[fd].ptrfiletable->readoffset),read_size);

        //(UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + read_size; 
        (UFDTArr[fd].ptrfiletable->readoffset) = 0; 
    }
    else
    {
        strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + ( UFDTArr[fd].ptrfiletable->readoffset),iSize);

        //(UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + iSize; 
        (UFDTArr[fd].ptrfiletable->readoffset) = 0; 
    }

    return iSize;
}

int WriteFile(int fd, char *arr, int iSize)
{
    if(((UFDTArr[fd].ptrfiletable->mode) != WRITE) && ((UFDTArr[fd].ptrfiletable->mode) != READ + WRITE))   return -1;

    if(((UFDTArr[fd].ptrfiletable->ptrinode->permission) != WRITE) && ((UFDTArr[fd].ptrfiletable->ptrinode->permission) != READ + WRITE))   return -1;

    if((UFDTArr[fd].ptrfiletable->writeoffset) == MAXFLESIZE)   return -2;

    if((UFDTArr[fd].ptrfiletable->ptrinode->FileType) != REGULAR)   return -3;

    strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + ( UFDTArr[fd].ptrfiletable->writeoffset),arr,iSize);

    (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + iSize;

    (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + iSize;

    return iSize;
}

int OpenFile(char * name, int mode)
{
    int i = 0;
    PINODE temp = NULL;

    if(name == NULL || mode <= 0)
        return -1;

    temp = Get_INODE(name);
    if(temp == NULL)
        return -2;

    if(temp->permission < mode)
        return -3;

    while(i < 50)
    {
        if(UFDTArr[i].ptrfiletable == NULL)
            break;
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
    if(UFDTArr[i].ptrfiletable == NULL)     return -1;
    UFDTArr[i].ptrfiletable->count = 1;
    UFDTArr[i].ptrfiletable->mode = mode;
    if(mode == READ + WRITE)
    {
        UFDTArr[i].ptrfiletable->readoffset = 0;
        UFDTArr[i].ptrfiletable->writeoffset = 0;
    }
    else if( mode == READ)
    {
        UFDTArr[i].ptrfiletable->readoffset = 0;
    }
    else if( mode == WRITE)
    {
        UFDTArr[i].ptrfiletable->writeoffset = 0;
    }
    UFDTArr[i].ptrfiletable->ptrinode = temp;
    (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)++;

    return i;
}

void CloseFileByName(int fd)
{
    UFDTArr[fd].ptrfiletable->readoffset = 0;
    UFDTArr[fd].ptrfiletable->writeoffset = 0;
    (UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount)--;
}

int CloseFileByName(char * name)
{
    int i = 0;
    i = GetFDFromName(name);
    if(i == -1)
        return -1;
    
    UFDTArr[i].ptrfiletable->readoffset = 0;
    UFDTArr[i].ptrfiletable->writeoffset = 0;
    (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;

    return 0;
}


void ls_file()
{
    int i = 0;
    PINODE temp = head;

    if(SUPERBLOCKobj.FreeInode == MAXINODE)
    {
        printf("Error : There are no files\n");
        return;
    }

    printf("\nFile Name\tInode Number\tFile Size\tLink count\n");
    printf("--------------------------------------------------------------\n");
    while(temp != NULL)
    {
        if(temp->FileType != 0)
        {
            printf("%s\t\t%d\t\t%d\t\t%d\n",temp->FileName, temp->InodeNumber, temp->FileActualSize, temp->LinkCount);
        }
        temp = temp->next;
    }
    printf("--------------------------------------------------------------\n");
}

int stat_file(char *name)
{
    PINODE temp = head;
    int i = 0;

    if(name == NULL)    return -1;

    while(temp != NULL)
    {
        if(strcmp(name,temp->FileName) == 0)
            break;
        temp = temp->next;
    }

    if(temp == NULL)    return -2;

    printf("\n-----------Statistical Information about file-----------\n");
    printf("File name : %s\n",temp->FileName);
    printf("Inode Number: %d \n",temp->InodeNumber);
    printf("File Size : %d bytes\n",temp->FileSize);
    printf("Actual file size : %d bytes\n",temp->FileActualSize);
    printf("Link Count : %d\n",temp->LinkCount);
    printf("Reference count : %d\n",temp->ReferenceCount);

    if(temp->permission == 1)
        printf("File permission : Read only\n");
    else if(temp->permission == 2)
        printf("File permission : Write only\n");
    else if(temp->permission == 1)
        printf("File permission : Read & Write\n");
    printf("---------------------------------------------------------------\n\n");

    return 0;
}

int main()
{
    char *ptr = NULL;
    int ret = 0, fd = 0, count = 0;
    char command[4][80],str[80],arr[1024];

    InitialiseSuperBlock();
    CreateDILB();

    while(1)
    {
            fflush(stdin);

            printf("\nCVFS : >>>> ");

        fgets(str,80,stdin);
  
        count = sscanf(str,"%s%s%s%s",command[0],command[1],command[2],command[3]);
		printf("count(main): %d\n",count);

            if(count == 1)
            {
                if(strcmp(command[0],"ls") == 0)
                {
                    ls_file();
                }
                else if(strcmp(command[0],"clear") == 0)
                {
                    system("clear");
                    continue;
                }
                else if(strcmp(command[0],"help") == 0)
                {
                    DisplayHelp();
                    continue;
                }
                else if(strcmp(command[0],"exit") == 0)
                {
                    printf("Terminating the Virtual File System\n");
                    break;
                }
                else 
                {
                    printf("\nERROR : Command not found !!!\n");
                    continue;
                }
            }
            else if(count == 2)
            {
                if(strcmp(command[0],"stat") == 0)
                {
                    ret = stat_file(command[1]);
                    if(ret == -1)
                        printf("ERROR : Incorrect parameters\n");
                    if(ret == -2)
                        printf("ERROR : There is no such file\n");
                    continue;
                }
                else if(strcmp(command[0],"close") == 0)
                {
                    ret = CloseFileByName(command[1]);
                    if(ret == -1)
                        printf("ERROR : There is no such file\n");
                    continue;
                }
                else if(strcmp(command[0],"rm") == 0)
                {
                    ret = rm_File(command[1]);
                    if(ret == -1)
                        printf("ERROR : There is no such file\n");
                    continue;
                }
                else if(strcmp(command[0],"man") == 0)
                {
                    man(command[1]);
                }
                else if(strcmp(command[0],"write") == 0)
                {
                    fd = GetFDFromName(command[1]);
                    if(fd == -1)
                    {
                        printf("ERROR : Incorrect parameters\n");
                        continue;
                    }
                    printf("Enter the data : \n");
                    scanf("%[^'\n']s",arr);
                    fflush(stdin);
                    
                    ret = strlen(arr);
                    if(ret == 0)
                    {
                        printf("ERROR : Incorrect parameters\n");
                        continue;
                    }
                    ret = WriteFile(fd, arr, ret);
                    if(ret == -1)
                        printf("ERROR : Permission denied \n");
                    if(ret == -2)
                        printf("ERROR : There is no sufficient memory to write \n");
                    if(ret == -3)
                        printf("ERROR : It is not regular file \n");
		
                	fgets(str,80,stdin);
                    
                }
                else 
                {
                    printf("\nERROR : Command not found !!!\n");    
                    continue;
                }
            }
            else if(count == 3)
            {
                if(strcmp(command[0],"create") == 0)
                {
                    ret = CreateFile(command[1],atoi(command[2]));
                    if(ret >= 0)
                        printf("File is succesfully created with the file descriptor : %d\n",ret);
                    if(ret == -1)
                        printf("ERROR : Incorrect parameters\n");
                    if(ret == -2)
                        printf("ERROR : There is no inodes\n");
                    if(ret == -3)
                        printf("ERROR : File already exists\n");
                    if(ret == -4)
                        printf("ERROR : Memory allocation failure\n");
                    continue;
                }
                else if(strcmp(command[0],"open") == 0)
                {
                    ret = OpenFile(command[1],atoi(command[2]));
                    if(ret >= 0)
                        printf("File is succesfully opned with the file descriptor : %d\n",ret);
                    if(ret == -1)
                        printf("ERROR : Incorrect parameters\n");
                    if(ret == -2)
                        printf("ERROR : This file is not present\n");
                    if(ret == -3)
                        printf("ERROR : Permission denied\n");    
                    continue;
                }
                else if(strcmp(command[0],"read") == 0)
                {
                    fd = GetFDFromName(command[1]);
                    if(fd == -1)
                    {    
                        printf("ERROR : Incorrect parameters\n");
                        continue;
                    }
                    ptr = (char *)malloc(sizeof(atoi(command[2])) +1);
                    if(ptr == NULL)
                    {
                        printf("ERROR : Memory allocation failure\n");
                        continue;
                    }
                    ret = ReadFile(fd,ptr,atoi(command[2]));
                    if(ret == -1)
                        printf("ERROR : This file is not present\n");
                    if(ret == -2)
                        printf("ERROR : Permission denied\n");
                    if(ret == -3)
                        printf("ERROR : Reached at end of the file\n"); 
                    if(ret == -4)
                        printf("ERROR : It is not a regular file\n"); 
                    if(ret == 0)
                        printf("ERROR : File empty\n");    
                    if(ret > 0)
                    {
                        write(1,ptr,ret);
                    }
                    continue;
                }
                else 
                {
                    printf("\nERROR : Command not found !!!\n");
                    continue;
                }
            }
            else
            {
		printf("count(else): %d\n",count);
                printf("\nERROR : Command not found !!!\n");
                continue;
            }

    }
    return 0;
}









































