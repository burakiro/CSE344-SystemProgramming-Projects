#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>

#define MAX_ALLPOSSIBLE_INPUTS_COUNT 1000
#define WORD_HAS_MAX_LETTER 100
#define BLKSIZE 1024


struct Paranthesis{
    char inside[50];
    int size;
};

char allPossibleInputs[MAX_ALLPOSSIBLE_INPUTS_COUNT][WORD_HAS_MAX_LETTER];
int startPossibleInputsIndex=0;
int allLineSizes[MAX_ALLPOSSIBLE_INPUTS_COUNT];
int numOfLines;


int isEndWithDollarSymbol(char commandInput[]);
int isStartWithASymbol(char commandInput[]);
void stringReplacer(char commandInput[], char ***allFileIn3DArr, int fd,char *fileName);
void recurParanthesisProcessor(char *clearInput, int inputSize, char *new, int newIndex,int index,struct Paranthesis parantheses[WORD_HAS_MAX_LETTER], int structSize);
int isMatch(char *word1, char *word2);
int isMatchWithI(char *word1, char *word2);
void arrayClear(char *inputArr);
int findSize(char *willFindSize);
void myParantehesisSeperater(struct Paranthesis paratheses[WORD_HAS_MAX_LETTER], char inputString[]);
char *** fileStringReader(char *inputArr);
void twoDimArrayClear(char inputArr[MAX_ALLPOSSIBLE_INPUTS_COUNT][WORD_HAS_MAX_LETTER]);
int main(int argc, char *argv[]){
    char *buffer = malloc(sizeof(char) * BLKSIZE * BLKSIZE * BLKSIZE);
    int fromfd;

    fromfd = open(argv[2], O_RDWR);
    if(fromfd == -1){
        perror("Failed to open input file!");
        return 1;
    }

    read (fromfd, buffer, 1024*1024*1024);
    char ***allFileIn3DArr;
    allFileIn3DArr=fileStringReader(buffer);
    free(buffer);
    stringReplacer(argv[1],allFileIn3DArr,fromfd, argv[2]);

    for(int i=0; i<BLKSIZE;i++){
        for(int j=0;j<BLKSIZE;j++){
            free(allFileIn3DArr[i][j]);
        }
        free(allFileIn3DArr[i]);
    }
    
    free(allFileIn3DArr);
    return 0;
}


int isEndWithDollarSymbol(char commandInput[]){
int i=0;
    for(int slashCount=0;slashCount!=2;i++){
        if(commandInput[i] == '/'){
            slashCount++;
        }
    }
    if(commandInput[i-2] == '$'){
        return 1;
    }
    else{
        return 0;
    }

}

int isStartWithASymbol(char commandInput[]){

    if(commandInput[1]=='^'){
        return 1;
    }
    else{
        return 0;
    }

}

void stringReplacer(char commandInput[], char ***allFileIn3DArr,int fd,char *fileName){
    struct flock lock;
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;
    fcntl(fd,F_SETLK,&lock);

    if(fcntl(fd,F_SETLK,&lock) == -1){
        perror("This application locked by another process!");
        exit(1);
    }

    char firstWord[WORD_HAS_MAX_LETTER];
    for(int i=0; i<WORD_HAS_MAX_LETTER;i++){
        firstWord[i] = '\0';
    }
    char secondWord[WORD_HAS_MAX_LETTER];
    for(int i=0; i<WORD_HAS_MAX_LETTER;i++){
        secondWord[i] = '\0';
    }
    int slashCounter=0;
    int fileIndex=0;
    struct Paranthesis myReads[WORD_HAS_MAX_LETTER];
    if(commandInput[fileIndex] != '/'){
        printf("Wrong Command Line Argument!\n");
        printf("Example Usage: ./deneme '/s[tr]*1/str2/i' inputFilePath\n");
        exit(1);
    }

    if(commandInput[fileIndex+1] == '*' || commandInput[fileIndex+1] == '/'){
        printf("Wrong Command Line Argument!\n");
        printf("Example Usage: ./deneme '/s[tr]*1/str2/i' inputFilePath\n");
        exit(1);
    }

    int slashChecker = 0;
    for(int p = 0; p<BLKSIZE;p++){
        if(commandInput[p] == '/'){
            slashChecker++;
        }
    }

    if(slashChecker % 3 != 0){
        printf("Wrong Command Line Argument!\n");
        printf("Example Usage: ./deneme '/s[tr]*1/str2/i' inputFilePath\n");
        exit(1); 
    }    

    for(int fIndex=0, sIndex=0;slashCounter!=3;fileIndex++){
    if(commandInput[fileIndex] == '/'){
        slashCounter++;
        continue;
    } 
    if(slashCounter<2 && (commandInput[fileIndex]!= '^' && commandInput[fileIndex] != '$')){
        firstWord[fIndex] = commandInput[fileIndex];
        fIndex++;
    }
    if(slashCounter==2){
        secondWord[sIndex] = commandInput[fileIndex];
        sIndex++;
    } 

    }
    
    myParantehesisSeperater(myReads,firstWord);
    if(isStartWithASymbol(commandInput) == 1 && isEndWithDollarSymbol(commandInput) == 0){
        for(int i=0;i<numOfLines;i++){
            for(int j=0;j<MAX_ALLPOSSIBLE_INPUTS_COUNT;j++){     
                if(commandInput[fileIndex]=='i'){
                    if(isMatchWithI(allPossibleInputs[j],allFileIn3DArr[i][0])){
                        for(int k=0; k<allLineSizes[i];k++){
                        for(int l=0; l<MAX_ALLPOSSIBLE_INPUTS_COUNT;l++){
                            if(isMatchWithI(allPossibleInputs[l],allFileIn3DArr[i][k]) && strcmp(allFileIn3DArr[i][k],"\0")!=0){
                                strcpy(allFileIn3DArr[i][k],secondWord);     
                            }
                        }
                        
                    }
                    }
                }         
                else if(isMatch(allPossibleInputs[j],allFileIn3DArr[i][0])){
                    for(int k=0; k<allLineSizes[i];k++){
                        for(int l=0; l<WORD_HAS_MAX_LETTER;l++){
                            if(isMatch(allPossibleInputs[l],allFileIn3DArr[i][k])&& strcmp(allFileIn3DArr[i][k],"\0")!=0){
                                    strcpy(allFileIn3DArr[i][k],secondWord);                              
                            }
                        }
                        
                    }
                }
            }


        }
    }
    if(isStartWithASymbol(commandInput) == 0 && isEndWithDollarSymbol(commandInput) == 1){
        for(int i=0;i<numOfLines;i++){
            for(int j=0;j<MAX_ALLPOSSIBLE_INPUTS_COUNT;j++){     
                if(commandInput[fileIndex]=='i'){
                    if(isMatchWithI(allPossibleInputs[j],allFileIn3DArr[i][allLineSizes[i] - 1])){
                        for(int k=0; k<allLineSizes[i];k++){
                        for(int l=0; l<MAX_ALLPOSSIBLE_INPUTS_COUNT;l++){
                            if(isMatchWithI(allPossibleInputs[l],allFileIn3DArr[i][k])&& strcmp(allFileIn3DArr[i][k],"\0")!=0 ){                          
                            }
                        }
                        
                    }
                    }
                }         
                else if(isMatch(allPossibleInputs[j],allFileIn3DArr[i][allLineSizes[i] - 1])){
                    for(int k=0; k<allLineSizes[i];k++){
                        for(int l=0; l<MAX_ALLPOSSIBLE_INPUTS_COUNT;l++){
                            if(isMatch(allPossibleInputs[l],allFileIn3DArr[i][k])&& strcmp(allFileIn3DArr[i][k],"\0")!=0){
                                strcpy(allFileIn3DArr[i][k],secondWord);
                            }
                        }
                        
                    }
                }
            }


        }
    }
    if(isStartWithASymbol(commandInput) == 1 && isEndWithDollarSymbol(commandInput) == 1 ){
        for(int i=0;i<numOfLines;i++){
            for(int j=0;j<MAX_ALLPOSSIBLE_INPUTS_COUNT;j++){     
                if(commandInput[fileIndex]=='i'){
                    if(isMatchWithI(allPossibleInputs[j],allFileIn3DArr[i][allLineSizes[i] - 1])){
                        for(int q=0;q<numOfLines;q++){
                            for(int p=0; p<MAX_ALLPOSSIBLE_INPUTS_COUNT;p++){
                                if(isMatchWithI(allPossibleInputs[p],allFileIn3DArr[q][0]) && q == i){
                                    for(int k=0; k<allLineSizes[i];k++){
                                        for(int l=0; l<MAX_ALLPOSSIBLE_INPUTS_COUNT;l++){
                                            if(isMatchWithI(allPossibleInputs[l],allFileIn3DArr[i][k])){
                                                if(findSize(allFileIn3DArr[i][k]) == 0){break;}
                                                        strcpy(allFileIn3DArr[i][k],secondWord);                                                
                                            }
                                        }
                                     }
                                }
                            }
                        }
                    }
                }
                           
                
                 else if(isMatch(allPossibleInputs[j],allFileIn3DArr[i][allLineSizes[i]-1])){
                        for(int q=0;q<numOfLines;q++){
                            for(int p=0; p<MAX_ALLPOSSIBLE_INPUTS_COUNT;p++){
                                if(isMatch(allPossibleInputs[p],allFileIn3DArr[q][0]) && q == i){
                                    for(int k=0; k<allLineSizes[i];k++){
                                        for(int l=0; l<MAX_ALLPOSSIBLE_INPUTS_COUNT;l++){
                                            if(isMatch(allPossibleInputs[l],allFileIn3DArr[i][k])){   
                                                if(findSize(allFileIn3DArr[i][k]) == 0){break;}
                                                    strcpy(allFileIn3DArr[i][k],secondWord);       
                                            }
                                        }
                                    }
                                }
                         }
                     }  
                 }
            }
        }
    }        
    if(isStartWithASymbol(commandInput) == 0 && isEndWithDollarSymbol(commandInput) == 0){
        for(int i=0;i<numOfLines;i++){
            for(int j=0;j<allLineSizes[i];j++){     
                    for(int k=0; k<MAX_ALLPOSSIBLE_INPUTS_COUNT;k++){
                        if(commandInput[fileIndex] == 'i'){
                             if((isMatchWithI(allPossibleInputs[k], allFileIn3DArr[i][j])) && (strcmp(allFileIn3DArr[i][j], "\0") !=0)){
                            strcpy(allFileIn3DArr[i][j],secondWord);
                        }
                        }
                        else{
                            if((isMatch(allPossibleInputs[k], allFileIn3DArr[i][j])) && (strcmp(allFileIn3DArr[i][j], "\0") !=0)){
                                strcpy(allFileIn3DArr[i][j],secondWord);                         
                        }
                    }

                }
                       
            }
        }                   
    }
    
    int oneDArrIndex=0;
    char *oneDArray= malloc(sizeof(char) * BLKSIZE * BLKSIZE * BLKSIZE);
    for(int i=0;i<numOfLines;i++){
        for(int j=0;j<allLineSizes[i];j++){
            for(int k=0;k<WORD_HAS_MAX_LETTER;k++){
                if(allFileIn3DArr[i][j][k] == '\0'){
                    break;
                }
                oneDArray[oneDArrIndex] = allFileIn3DArr[i][j][k];
                oneDArrIndex++;
            }
            if(j != allLineSizes[i]-1){
            oneDArray[oneDArrIndex] = ' ';
            oneDArrIndex++;
            }          
        }
        oneDArray[oneDArrIndex]='\n';
        oneDArrIndex++;
    }

    lock.l_type = F_UNLCK;
    fcntl(fd,F_SETLK,&lock);
    close(fd);
    fd = open(fileName, O_TRUNC | O_RDWR);

    lock.l_type = F_WRLCK;
    fcntl(fd,F_SETLK,&lock);
    if(fcntl(fd,F_SETLK,&lock) == -1){
        exit(1);
    }
    write(fd,oneDArray,strlen(oneDArray));     

    if(commandInput[fileIndex] ==';' || commandInput[fileIndex+1] == ';'){
        if(commandInput[fileIndex] == ';'){
            twoDimArrayClear(allPossibleInputs);
            startPossibleInputsIndex=0;
            stringReplacer(commandInput+(fileIndex+1),allFileIn3DArr,fd,fileName);
        }

        if(commandInput[fileIndex + 1] == ';' ){
            twoDimArrayClear(allPossibleInputs);
            startPossibleInputsIndex=0;
            stringReplacer(commandInput+(fileIndex+2),allFileIn3DArr,fd,fileName);

        }

    }

    lock.l_type = F_UNLCK;
    fcntl(fd,F_SETLK,&lock);
    free(oneDArray);  
}

int isMatch(char *word1, char *word2){
    if(findSize(word1) == 0 ) {
        if (findSize(word2) == 0)
        {
            return 1;
        }  
        return 0;
    }

    if(word1[1] != '*' ){
        if(word1[0] == word2[0] && isMatch(&(*(word1 + 1)), &(*(word2 + 1)))){
            return 1;
        }
        return 0;
    }

    if(word1[1] == '*'){
        if(word1[0] == word2[0] && isMatch(&(*(word1)), &(*(word2 +1)))){
            return 1;
        }
        else if(word1[0] == word2[0] && isMatch(&(*(word1 + 2)), &(*(word2 + 1))))
            return 1;
        else if(word1[0] != word2[0] && isMatch(&(*(word1 + 2)), &(*(word2)))){
            return 1;
        } 
        else {
            return 0;
        }
    }
    else{
        return 0;
    }
}

int isMatchWithI(char *word1, char *word2){
    if(findSize(word1) == 0 ) {
        if (findSize(word2) == 0)
        {
            return 1;
        }  
        return 0;
    }

    if(word1[1] != '*' ){
        if((word1[0] == word2[0] || (int)(word1[0]) - (int)(word2[0]) == -32 || (int)(word1[0]) - (int)(word2[0]) == 32 ) && isMatchWithI(&(*(word1 + 1)), &(*(word2 + 1)))){
            return 1;
        }
        return 0;
    }

    if(word1[1] == '*'){
        if((word1[0] == word2[0] || (int)(word1[0]) - (int)(word2[0]) == -32 || (int)(word1[0]) - (int)(word2[0]) == 32 ) && isMatchWithI(&(*(word1)), &(*(word2 +1)))){
            return 1;
        }
        else if((word1[0] == word2[0] || (int)(word1[0]) - (int)(word2[0]) == -32 || (int)(word1[0]) - (int)(word2[0]) == 32 ) && isMatchWithI(&(*(word1 + 2)), &(*(word2 + 1))))
            return 1;
        else if(((int)(word1[0]) - (int)(word2[0]) != -32 && (int)(word1[0]) - (int)(word2[0]) != 32 ) && isMatchWithI(&(*(word1 + 2)), &(*(word2)))){
            return 1;
        } 
        else{
            return 0;
        } 
            
    }

    else{
        return 0;
    }
}

char *** fileStringReader(char *inputArr){
    int xIndexOf3DArr=0;
    int yIndexOf3DArr=0;
    int wordDetectorIndex=0;
    char wordDetector[WORD_HAS_MAX_LETTER];
    arrayClear(wordDetector);
    char ***allFileIn3DArr;
    allFileIn3DArr = malloc(sizeof(char **)* BLKSIZE);
    for (int i = 0; i < BLKSIZE; i++)
    {
        allFileIn3DArr[i] = malloc(sizeof(char *)* BLKSIZE);
        for (int j = 0; j < BLKSIZE; j++)
        {
            allFileIn3DArr[i][j] = malloc(sizeof(char)* BLKSIZE);
        }

    }
    

    
    for(int i=0;inputArr[i]!='\0';i++){
        if(inputArr[i]==' '){
            strcpy(allFileIn3DArr[xIndexOf3DArr][yIndexOf3DArr],wordDetector);
            arrayClear(wordDetector);
            wordDetectorIndex=0;
            yIndexOf3DArr++;
            continue;
        }

        else if(inputArr[i]=='\n'){
            strcpy(allFileIn3DArr[xIndexOf3DArr][yIndexOf3DArr],wordDetector);
            arrayClear(wordDetector);
            wordDetectorIndex=0;
            allLineSizes[xIndexOf3DArr]=yIndexOf3DArr+1;
            xIndexOf3DArr++;
            yIndexOf3DArr=0;
            continue;
        }

        else{            
            wordDetector[wordDetectorIndex]=inputArr[i];
            wordDetectorIndex++;
        }
            
    }
    numOfLines=xIndexOf3DArr;

    /*printf("Array is printing\n");
    for (int i = 0; i < numOfLines; i++)
    {
        for (int j = 0; j < allLineSizes[i]; j++)
        {
            printf("%s ",allFileIn3DArr[i][j]);
        }
        printf("\n");
    }*/
    return allFileIn3DArr;
}

void arrayClear(char *inputArr){

    for(int i=0;i<WORD_HAS_MAX_LETTER;i++){
        inputArr[i] = '\0';
    }
}

void twoDimArrayClear(char inputArr[MAX_ALLPOSSIBLE_INPUTS_COUNT][WORD_HAS_MAX_LETTER]){
    for(int i=0;i<MAX_ALLPOSSIBLE_INPUTS_COUNT;i++){
        for(int j=0;j<WORD_HAS_MAX_LETTER;j++){
            inputArr[i][j] = '\0';
        }
    }

}

void myParantehesisSeperater(struct Paranthesis parantheses[WORD_HAS_MAX_LETTER], char inputString[]){
    char clearInput[WORD_HAS_MAX_LETTER];
    char newArray[WORD_HAS_MAX_LETTER];
    int j=0,clearInputSize=0;
    int structSize = 0;


    for(int i=0;i<WORD_HAS_MAX_LETTER;i++){
        if(inputString[i] == '['){
            clearInput[j]='~';
            for(int k=i+1,l=0;inputString[k]!= ']'; k++){
                parantheses[structSize].inside[l] = inputString[k];
                parantheses[structSize].size=l+1;
                l++;
            }
            structSize++;
            i=i+parantheses[structSize-1].size + 1;
            j++;
        }
        if(inputString[i] == ']'){
            continue;
        }
        else{
            clearInput[j] = inputString[i];
            j++;
        }

    }
        clearInput[j] = '\0';

        clearInputSize = findSize(clearInput);
        

        recurParanthesisProcessor(clearInput,clearInputSize,newArray,0,0,parantheses,-1);
}

void recurParanthesisProcessor(char *clearInput, int inputSize, char *new, int newIndex,int index,struct Paranthesis parantheses[50], int structSize){
    
    if(index == inputSize){
        strcpy(allPossibleInputs[startPossibleInputsIndex], new);
        startPossibleInputsIndex++;

        return;
    }

    if(clearInput[index] != '~'){
        new[newIndex] = clearInput[index];
        newIndex++;
        index++;
        recurParanthesisProcessor(clearInput,inputSize,new,newIndex,index, parantheses,structSize);
    }
    else{
        structSize++;
        int i=0;
        while(i<parantheses[structSize].size){
            new[newIndex] = parantheses[structSize].inside[i];
            newIndex++;
            index++;
            recurParanthesisProcessor(clearInput,inputSize,new,newIndex,index, parantheses,structSize);
            newIndex--;
            index--;
            i++;
        }
        
    }

}

int findSize(char *willFindSize){

    int sizeCounter=0;

    for(int i=0; willFindSize[i] != '\0'; i++){
        sizeCounter++;
    }

    return sizeCounter;

}