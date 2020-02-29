#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <regex>
#include <fstream>
#include <unistd.h>
#include <sstream>
#include <set>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <stdlib.h>
#include <sys/mman.h>
#include <iomanip>


using vec = std::vector<std::string>;

vec split(std::string input, const std::string& regex);
void merge_sort (int* number, int l, int r);
void merge(int* number, int l,int m, int r);

int main(int argc, char** argv)
{   
    using namespace std;

    /*
    cout <<  "THIS PART JUST FOR LEARNING" << endl;
    cout << "Number of argument: " << argc << endl;
    for (int i = 0; i < argc; i++)
    {
        cout << argv[i] << endl;


    }
    cout << "END LEARNING \n \n";
    */

    if (argc < 2){
        cout << "Invalid command!!! Try main.out + *.txt + delimiter" << endl;
        exit(-1);
    }
    else {
        ifstream inFile(argv[1]);
        if (inFile.fail()) {
            cerr << "Unable to open file for reading. Check if file name is correct!!!" << endl;
            exit(1);
        }
        // save the whole text file contents into a string
        string file_contents {istreambuf_iterator<char>(inFile), istreambuf_iterator<char>()};
        
        // remove all lines from string
        //file_contents.erase(std::remove(file_contents.begin(), file_contents.end(), '\r'), file_contents.end());
        //file_contents.erase(std::remove(file_contents.begin(), file_contents.end(), '\n'), file_contents.end());
        
        // remove all delimiter
        vector<string>dataVector = split(file_contents, argv[2]);
        inFile.close();
        int* numVector = new int[dataVector.size()];
        
        cout << "#############################################################" << endl;
        cout << "#############################################################" << endl;
        cout << "   #######################################################   " << endl;
        cout << "      #################################################      " << endl;
        cout << endl;
        cout << "This is original file content: " << endl;
        cout << "\t" << file_contents << endl;
        cout << endl;
        cout << endl;
        cout << "This is the file content without delimiter: " << endl;
        cout << "\t" ;
        for (int i = 0; i < dataVector.size(); i++) {
            cout <<  dataVector[i] << " ";
            int num;
            
            try{
                num = stoi(dataVector[i]);
            }
            catch(const std::invalid_argument& e){
                std::cout << "\n";
                std::cout << "\n ERROR: File contains invalid data or wrong delimiter. Please check the test file or delimiter argument." <<std::endl;
                exit(-1);
            }
            numVector[i] = num;
        }
        cout << endl;
        cout << endl;
    
        int shareMemoryID;
        int* numVectorShared = nullptr;
        int size = dataVector.size();
        
        // CREATE SHARE MEMORY USING MMAP
        size_t ShareMemoryArray = sizeof(int)*size;
        numVectorShared = static_cast<int*> (mmap(NULL, sizeof(int) *size, PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS,-1,0));


        
        // COPY DATA TO SHARE MEMORY
        for (int i = 0; i < size; i++){
            numVectorShared[i] = numVector[i];
        }
        cout << endl;
        cout << "\tLeft Child \t\t ProcessID \t\t Right Child \t\t ProcessID" << endl;
        merge_sort(numVectorShared,0, size -1);
        cout << "Sorted output (Merge Sort): " << endl;
        cout << "\t" ;
        for (int i = 0; i < dataVector.size(); i++) {
            cout << numVectorShared[i] << " ";
        }
        cout << endl;
        cout << endl;
        cout << "      ####################################################      " << endl;
        cout << "   ##########################################################   " << endl;
        cout << "################################################################" << endl;
        cout << "################################################################" << endl;
        
        delete[] numVector;
        //delete[] numVectorShared; 
        return 0;
        
    }
}
    
// split string or tokenizer method 
std::vector<std::string> split(std::string input, const std::string& delimiter)
{
    std::vector<std::string> list_number; 
    size_t pos = 0;
    std::string token;
    try {
        if (input.find(delimiter) == std::string::npos)
            throw std::invalid_argument("ERROR: Delimiter doesn't exist!!!");

        while ((pos = input.find(delimiter)) != std::string::npos) {
        token = input.substr(0, pos);
        list_number.push_back(token);
        input.erase(0, pos + delimiter.size());
        }
    }
    catch(const std::invalid_argument& e){
        std::cout << e.what() <<std::endl;
        exit(-1);
    }
    
    if (input.size() > 0)
    {
        list_number.push_back(input);
    }
    return list_number;
    
}

void merge_sort (int* number, int l, int r)
{
    //std::cout << "in sort" << std::endl;
    if (l<r) {
        int m = l + ((r - l))/2;
              
        
        // This is where the fork() is used
        pid_t lpid, rpid;
        lpid = fork();
        if (lpid < 0)
        {
            perror("Left fork() error!!!");
            _exit(-1);
        }
        else if (lpid == 0)
        {   
            std::cout << "\t";
            for (int i = l; i < m; i++){
                std::cout << number[i] << " ";
            }
            std::cout << "\t\t";
            printf("%10d",(getpid()));
            std::cout << std::endl;
            merge_sort(number, l, m);
            _exit(0);
        }
        
        rpid = fork();
        if (rpid < 0)
            {
            perror("Right fork() error!!!");
            _exit(-1);
            }
        else if (rpid == 0)
            {
            std::cout << "\t\t\t\t\t\t\t";
            for (int j = m+1; j <= r; j++){
                std::cout << number[j] << " ";
               
            }
            std::cout << "\t\t";
            printf("%10d",(getpid()));
            std::cout << std::endl; 
            merge_sort(number, m+1, r);
            _exit(0);
            }
        
             
        /*
        // regular merge
        merge_sort(number, l, m);
        merge_sort(number, m+1, r);
        */
        //Create a status value
        
        int statusPtr;
        waitpid(lpid,&statusPtr,0);
        waitpid(rpid,&statusPtr,0);
        /*
        for (int i = 0; i < r; i++){
            
            std::cout << number[i] << "|";
        }
            std::cout << std::endl;
            */
        
        merge(number,l,m,r);
    }
}

void merge(int* number, int l,int m, int r)
{
    //std::cout << "in merge" << std::endl;
    // Create 2 temp array
    //int L[m],R[r-m];

    // Create 2 temp array on the heap
    int* L = new int[m-l+1];
    int* R = new int[r-m+1];

    int i, j, k;
    int m1 = m - l + 1;
    int m2 = r - m;
    for (i = 0; i < m1;i++) {
        L[i] = number[l + i];
    }
    for (j = 0; j < m2;j++){
        R[j] = number[m+j+1];
    }
    i = 0; 
    j = 0;
    k = l; 
    while (i < m1 && j < m2){
        if (L[i] < R[j]){
            number[k] = L[i];
            i++;
            
        }
        else if(R[j] <= L[i]){
            number[k] = R[j];
            j++;
    
        }
        k++;
        
    }
    while (i < m1)
    {
        number[k] = L[i];
        i++;
        k++;
       
      
    }
    while (j < m2)
    {
        
        number[k] = R[j];
        j++;
        k++;
    }
    // Free 2 array memory
    delete[] L;
    delete[] R;
}



  



  

