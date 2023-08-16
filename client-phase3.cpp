#include<bits/stdc++.h>
#include<fstream>
#include<sys/socket.h>
#include<sys/sendfile.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/time.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<dirent.h>
#include<openssl/md5.h>
#include<sys/mman.h>

using namespace std;

void md5(char* filepath,char* ans){
    unsigned char result[MD5_DIGEST_LENGTH];
    int file_descript;
    unsigned long file_size;
    char* file_buffer;
    file_descript = open(filepath, O_RDONLY);
    if(file_descript < 0) exit(-1);
    struct stat statbuf;
    if(fstat(file_descript, &statbuf) < 0) exit(-1);
    file_size = statbuf.st_size;
    file_buffer = static_cast<char*>(mmap(0, file_size, PROT_READ, MAP_SHARED, file_descript, 0));
    MD5((unsigned char*) file_buffer, file_size, result);
    munmap(file_buffer, file_size); 
    char *ptr = &ans[0];
    int i;
    for(i=0; i <MD5_DIGEST_LENGTH; i++) {
    ptr += sprintf(ptr,"%02X",result[i]);
    }
    ans[2*MD5_DIGEST_LENGTH] = '\0';
}

void save_file(string p, string n, int s, int socket){
    ofstream file;
    file.open(p + n, ios::trunc|ios::out);
    char input[1024];
    while(s > 0){
        int len = recv(socket, input, 1024, 0);
        file.write(input, len);
        s -= len;
    }
    return;
}

void send_file(string p, string n, int s, int socket){
    string file = p + n;
    int descript = open(file.c_str(),0);
    sendfile(socket, descript, 0, s);
}

int GetFileSize(std::string filename){
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

string convertToString(char* a, int b, int e){
    int i;
    string s = "";
    for (i = b; i < e; i++) {
        s = s + a[i];
    }
    return s;
}

int give_socket(int l[][2], int j, int nn){
    int p;
    for(p = 0; p < nn; p++){
        if(l[p][0] == j){
            return l[p][1];
        }
    }
    return 0;
}

bool con_com(bool* a, int nn){
    for(int c = 0; c < nn; c++){
        if(!a[c]){
            return false;
        }
    }
    return true;
}

int main (int argc, char *argv[]) {
    vector <string> of_list;
    vector <int> of_size;
    DIR *dir;
    struct dirent *ent;
    string a = ".", b = "..", c = "Downloaded";
    string path = argv[2];
    string directory = path + c;
    bool e = false;

    if ((dir = opendir (argv[2])) != NULL){
        while ((ent = readdir (dir)) != NULL){
            string temp = ent->d_name;
            if(temp == c){
                e = true;
            }
            if (temp != a && temp != b && temp != c){
                of_list.push_back(temp);
            }
        }
        closedir (dir);
    }

    if( e == false){
        mkdir(directory.c_str(),0777);
    }

    sort(of_list.begin(),of_list.end());
    for (auto i = of_list.cbegin(); i != of_list.cend(); ++i){
        cout << *i << endl;
    }

    string cid, port, pid, i, j;
    char buffer[100], input[1024];
    int nn, fc;
    string line, filename;
    filename = argv[1];
    ifstream infile(filename);
    int n = 1;
    while (getline(infile, line)){
        if (n == 1){
            stringstream ss(line);
            ss >> cid >> port >> pid;
        }
        else if (n == 2){
            stringstream ss(line);
            ss >> i;
            nn = stoi(i);
            break;
        }
        n++;
    }

    int ninfo[2*nn];
    bool check_c[nn] = {false};
    bool con_estd[nn] = {false};
    int n_ids[nn][2];

    int n_socket[nn][2] = {-1};
    int l_socket[nn][2] = {-1};

    while (getline(infile, line)){
        if (n == 2){
            stringstream ss(line);
            for(int k = 0; k < 2*nn; k++){
                string temp;
                ss >> temp;
                ninfo[k] = stoi(temp);
            }
        }
        else if (n == 3){
            stringstream ss(line);
            ss >> j;
            fc = stoi(j);
            break;
        }
        n++;
    }

    string flist[fc][4];
    vector < string > fvec;

    while (getline(infile, line)){
        stringstream ss(line);
        string temp;
        ss >> temp;
        fvec.push_back(temp);
    }
    
    sort(fvec.begin(),fvec.end());
    for (int j = 0; j < fc; j++){
        flist[j][0] = fvec[j];
        flist[j][1] = "0";
        flist[j][2] = "0";
        flist[j][3] = "0";
    }

    for(int c = 0; c < cid.size(); c++){
        buffer[c] = cid[c];
    }
    buffer[cid.size()] = '~';
    for(int c = 0; c <  pid.size(); c++){
        buffer[c + 1 +  cid.size()] = pid[c];
    }
    buffer[cid.size() +  pid.size() + 1] = '.';
    
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 10000;
    int server_fd, max_id = -1;
    struct sockaddr_in address;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(stoi(port));
    bind(server_fd, (struct sockaddr *)&address,  sizeof(address));
    listen(server_fd,20);
    fd_set n_sock;

    while(!con_com(con_estd,nn)||!con_com(check_c,nn)){
        for(int j = 0; j < nn; j++){
            if(con_estd[j] != true){
                struct sockaddr_in s;
                s.sin_family = AF_INET;
                s.sin_addr.s_addr = INADDR_ANY; 
                s.sin_port = htons(ninfo[2*j+1]);
                int client = socket(AF_INET, SOCK_STREAM, 0);
                if(connect(client, (struct sockaddr *)&s,  sizeof(s)) >= 0){
                    con_estd[j] = true;
                    send(client, buffer, 100, 0);
                    n_socket[j][0] = ninfo[2*j];
                    n_socket[j][1] = client;
                }
                else{
                    close(client);
                }
            }
        }
        FD_ZERO(&n_sock);
        FD_SET(server_fd,&n_sock);
        max_id = server_fd;
        select(max_id + 1,&n_sock, NULL, NULL, &tv);

        for(int j = 0; j <= max_id; j++){
            if(j == server_fd){
                if(FD_ISSET(server_fd, &n_sock)){
                    int new_fd;
                    int addr_len =  sizeof(address);
                    new_fd = accept(server_fd,(struct sockaddr*)&address, (socklen_t*)&addr_len);
                    char temp[100];
                    int len = recv(new_fd,temp,100,0);
                    int k1 = 0;
                    while(temp[k1] != '~'){
                        k1++;
                    }
                    int c = stoi(convertToString(temp,0,k1));
                    int k2 = 0;
                    while(input[k2] != '.'){
                        k2++;
                    }
                    int u = stoi(convertToString(temp,k1 + 1,k2));
                    int r = -1;
                    for(int i=0;i<nn;i++){
                        if(ninfo[2*i]==c){
                            n_ids[i][0] = c;
                            n_ids[i][1] = u;
                            check_c[i] = true;
                            r = i;
                        }
                    }
                    l_socket[r][1] = new_fd;
                    l_socket[r][0] = ninfo[2*r];
                }
            }
        }
    }

    for(int i=0;i<nn;i++){
        cout<<"Connected to "<<ninfo[2*i]<<" with unique-ID "<<n_ids[i][1]<<" on port "<<ninfo[2*i+1]<<endl;
    }
        
    for(int j = 0; j < nn; j++){
        if(n_ids[j][1] < stoi(pid)){
            int s_sock = give_socket(n_socket,n_ids[j][0],nn);
            int r_sock = give_socket(l_socket,n_ids[j][0],nn);
            for(int k = 0; k < fc; k++){
                const char* msg = flist[k][0].c_str();
                send(s_sock,msg,flist[k][0].size(),0);
                int len = recv(r_sock, input, 1024, 0);
                if(input[0] == 'y'){
                    if(flist[k][1] == "0"){
                        flist[k][1] = to_string(n_ids[j][1]);
                        flist[k][2] = convertToString(input, 1, len);
                    }
                    else if(stoi(flist[k][1]) > n_ids[j][1]){
                        flist[k][1] = to_string(n_ids[j][1]);
                        flist[k][2] = convertToString(input, 1, len);
                    }
                }
            }
            char flag[1];
            flag[0] = 'd';
            send(s_sock,flag,1,0);
            while(true){
                int len = recv(r_sock, input, 100, 0);
                string file_name = convertToString(input, 0, len);
                if(file_name != convertToString(flag, 0, 1)){
                    string ans = "n";
                    for (auto i = of_list.cbegin(); i != of_list.cend(); ++i){
                        if(*i == file_name){
                            ans[0] = 'y';
                            ans += to_string(GetFileSize(path + '/' + file_name));
                        }
                    }
                    send(s_sock,ans.c_str(),ans.size(),0);
                }
                else{
                    break;
                }
            }
        }
        else{
            char flag[1];
            flag[0] = 'd';
            int s_sock = give_socket(n_socket,n_ids[j][0],nn);
            int r_sock = give_socket(l_socket,n_ids[j][0],nn);
            while(true){
                int len = recv(r_sock, input, 100, 0);
                string file_name = convertToString(input, 0, len);
                if(file_name != convertToString(flag, 0, 1)){
                    string ans = "n";
                    for (auto i = of_list.cbegin(); i != of_list.cend(); ++i){
                        if(*i == file_name){
                            ans[0] = 'y';
                            ans += to_string(GetFileSize(path + '/' + file_name));
                        }
                    }
                    send(s_sock,ans.c_str(),ans.size(),0);
                }
                else{
                    break;
                }
            }
            for(int k = 0; k < fc; k++){
                const char* msg = flist[k][0].c_str();
                send(s_sock,msg,flist[k][0].size(),0);
                int len = recv(r_sock, input, 1024, 0);
                if(input[0] == 'y'){
                    if(flist[k][1] == "0"){
                        flist[k][1] = to_string(n_ids[j][1]);
                        flist[k][2] = convertToString(input, 1, len);
                    }
                    else if(stoi(flist[k][1]) > n_ids[j][1]){
                        flist[k][1] = to_string(n_ids[j][1]);
                        flist[k][2] = convertToString(input, 1, len);
                    }
                }
            }
            send(s_sock,flag,1,0);  
        }
    }        

    for(int j = 0; j < nn; j++){
        if(n_ids[j][1] < stoi(pid)){
            int s_sock = give_socket(n_socket,n_ids[j][0],nn);
            int r_sock = give_socket(l_socket,n_ids[j][0],nn);
            for(int k = 0; k < fc; k++){
                const char* msg = flist[k][0].c_str();
                if(stoi(flist[k][1]) == n_ids[j][1]){
                    send(s_sock,msg,flist[k][0].size(),0);
                    save_file(path + "Downloaded/", flist[k][0], stoi(flist[k][2]), r_sock);
                    string p = path + "Downloaded/" + flist[k][0];
                    char ans[2*MD5_DIGEST_LENGTH+1];
                    char* fp = &p[0];
                    md5(fp,ans);
                    string hash(ans);
                    flist[k][3]= hash;
                }
            }
            
            char flag[1];
            flag[0] = 'd';
            send(s_sock,flag,1,0);

            while(true){
                int len = recv(r_sock, input, 100, 0);
                string file_name = convertToString(input, 0, len);
                if(file_name != convertToString(flag, 0, 1)){
                    send_file(path, file_name, GetFileSize(path + '/' + file_name), s_sock);
                }
                else{
                    break;
                }
            }
        }
        
        else{
            char flag[1];
            flag[0] = 'd';
            int s_sock = give_socket(n_socket,n_ids[j][0],nn);
            int r_sock = give_socket(l_socket,n_ids[j][0],nn);

            while(true){
                int len = recv(r_sock, input, 100, 0);
                string file_name = convertToString(input, 0, len);
                if(file_name != convertToString(flag, 0, 1)){
                    send_file(path, file_name, GetFileSize(path + '/' + file_name), s_sock);
                }
                else{
                    break;
                }
            }
            for(int k = 0; k < fc; k++){
                const char* msg = flist[k][0].c_str();
                if(stoi(flist[k][1]) == n_ids[j][1]){
                    send(s_sock,msg,flist[k][0].size(),0);
                    save_file(path + "Downloaded/", flist[k][0], stoi(flist[k][2]), r_sock);
                    string p = path + "Downloaded/" + flist[k][0];
                    char ans[2*MD5_DIGEST_LENGTH+1];
                    char* fp = &p[0];
                    md5(fp,ans);
                    string hash(ans);
                    flist[k][3]= hash;
                }
            }
            send(s_sock,flag,1,0);  
        }
    }
    
    for(int k = 0; k < fc; k++){
        transform(flist[k][3].begin(), flist[k][3].end(), flist[k][3].begin(), ::tolower);
        cout << "Found " << flist[k][0] << " at " << flist[k][1] << " with MD5 " << flist[k][3] << " at depth ";
        stoi(flist[k][1]) ? cout << 1 : cout << 0;
        cout << endl;
    }
}