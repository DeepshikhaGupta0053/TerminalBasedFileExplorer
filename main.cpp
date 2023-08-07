#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <bits/stdc++.h>
#include <pwd.h>
#include <grp.h>
#include <math.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <termios.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <algorithm>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;
bool FLAG = false;
bool SEARCH_FLAG = 0;
string s = "";
string path_returned_from_normal_mode;
int rows;
int cols;
stack<string> next_st;
stack<string> prev_st;
static void sig_handler(int sig);
void getWindowSize()
{
    struct winsize sizeofTerminal;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &sizeofTerminal);
    rows = sizeofTerminal.ws_row;
    cols=sizeofTerminal.ws_col;
}

string getHome()
{
    string homedir = getenv("HOME");
    return homedir;
}

void clearScreen()
{
    cout << "\033[H\033[2J\033[3J";
}

void displayDirectory(const char *directory_path);

char readKey()
{
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (nread == -1 && errno != EAGAIN)
        {
            cout << "Error : " << strerror(errno);
        }
    }

    if (c == '\x1b')
    {
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1)
            return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1)
            return '\x1b';
        if (seq[0] == '[')
        {
            switch (seq[1])
            {
            case 'A':
                return 'A'; // ARROW_UP;
                break;

            case 'B':
                return 'B'; // ARROW_DOWN;
                break;

            case 'C':
                return 'C';
                break;

            case 'D':
                return 'D';
                break;
            case 'H':
                return 'H';
                break;
            }
        }
        return '\x1b';
    }
    else
    {
        return c;
    }
}

string processKeypressForDisplay(int pointer, int size, string filename, const char *directory_path, int &upper, int &lower)
{
    char c = readKey();
    string totalpath;
    string directory_path_string = directory_path;
    string previous_path;
    int pos;

    switch (c)
    {

    case 'q':
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);
        exit(0);
        break;
    case 'A':
        if (pointer != 0)
        {
            pointer--;
        }

        if ((lower - pointer + 1) > (rows - 3))
        {
            upper--;
            lower--;
        }

        return to_string(pointer);
        break;
    case 'B':
        if (pointer < size - 1)
        {
            pointer++;
        }
        if ((pointer - upper + 1) > (rows - 3))
        {
            upper++;
            lower++;
        }

        return to_string(pointer);
        break;

    case 'C':
        if (!next_st.empty())
        {
            string temp = next_st.top();
            directory_path_string = directory_path;
            prev_st.push(directory_path_string);
            next_st.pop();
            displayDirectory(temp.c_str());
            return "-2";
        }

        break;

    case 'D':
        if (!prev_st.empty())
        {
            string temp = prev_st.top();
            directory_path_string = directory_path;
            next_st.push(directory_path_string);
            prev_st.pop();
            displayDirectory(temp.c_str());
            return "-2";
        }

        break;

    case 'H':
        directory_path_string = directory_path;
        prev_st.push(directory_path_string);
        displayDirectory(getHome().c_str());
        return "-2";
        break;

    case 'h':
        directory_path_string = directory_path;
        prev_st.push(directory_path_string);
        displayDirectory(getHome().c_str());
        return "-2";
        break;

    case 127:

        directory_path_string = directory_path;
        pos = directory_path_string.rfind('/');
        previous_path = directory_path_string.substr(0, pos);

        if (previous_path.size() == 0)
        {
            previous_path = "/";
        }

        // tracking movement backwards
        next_st.push(directory_path_string);

        displayDirectory(previous_path.c_str());
        return "-2";

        break;

    case ':':
        path_returned_from_normal_mode = directory_path_string;
        return "-2";
        break;

    case '\r':

        totalpath = directory_path;
        directory_path_string = directory_path;

        if (filename == ".")
        {
        }
        else if (filename == "..")
        {
            directory_path_string = directory_path;
            pos = directory_path_string.rfind('/');
            previous_path = directory_path_string.substr(0, pos);
            if (previous_path.size() == 0)
            {
                previous_path = "/";
            }

            // tracking movement backwards
            next_st.push(directory_path_string);

            displayDirectory(previous_path.c_str());
            return "-2";
        }
        else
        {
            totalpath += "/";
            totalpath += filename;
        }

        const char *totalpath_char = totalpath.c_str();

        struct stat file_stats;
        if (stat(totalpath_char, &file_stats) == -1)
        {
            cout << "Error : " << totalpath_char << strerror(errno) << endl;
            // return;
        }
        mode_t file_permission = file_stats.st_mode;
        if (S_ISDIR(file_permission))
        {
            prev_st.push(directory_path_string);
            displayDirectory(totalpath_char);
            return "-2";
        }

        if (S_ISREG(file_permission))
        {
            string vi = "gedit";
            char *vi_char = (char *)vi.c_str();
            pid_t pid = fork();
            if (pid == -1)
            {
                perror("fork");
            }

            if (pid == 0)
            {
                fstream fileStream;

                // fileStream.open(totalpath_char);
                // if (fileStream.fail())
                // {
                //     cout << "No permission to open file";
                // }

                if (open(totalpath_char, O_RDONLY) == -1)
                {
                    cout << "No permission to open file";
                }

                else
                {
                    string vi = "vi";
                    char *vi_char = (char *)vi.c_str();
                    const size_t n = strlen(totalpath_char); // excludes null terminator
                    char *b = new char[n + 1]{};             // {} zero initializes array
                    std::copy_n(totalpath_char, n, b);
                    char *exec_args[] = {vi_char, b, NULL};
                    execv("/usr/bin/vi", exec_args);
                }
            }
            if (pid > 0)
            {
                wait(0);
                // cout << "parent: " << pid << endl;
            }
        }

        break;
    }

    // have to decide what to return for other than currently defined cases
    return "0";
}
string formatFilesize(off_t fs)
{
    long double temp;
    string ans;
    stringstream ss;
    if (fs >= 1024)
    {
        temp = fs / 1024.0;
        if (temp < 1024)
        {
            ss << fixed << setprecision(2) << temp;
            ans = ss.str();
            ans += " KB";
            return ans;
        }
        else
        {
            temp = temp / 1024.0;
            if (temp < 1024)
            {
                ss << fixed << setprecision(2) << temp;
                ans = ss.str();
                ans += " MB";
                return ans;
            }
            else
            {
                temp = temp / 1024.0;
                if (temp < 1024)
                {
                    ss << fixed << setprecision(2) << temp;
                    ans = ss.str();
                    ans += " GB";
                    return ans;
                }
                else
                {
                    temp = temp / 1024.0;
                    if (temp < 1024)
                    {
                        ss << fixed << setprecision(2) << temp;
                        ans = ss.str();
                        ans += " TB";
                        return ans;
                    }
                }
            }
        }
    }

    ans = to_string(fs);
    ans += " B";
    return ans;
}

bool compareFilenames(const array<string, 6> &a1,
                      const array<string, 6> &a2)
{
    if (a1[5] <= a2[5])
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool compare(string s1, string s2)
{
    if (s1 <= s2)
    {
        return true;
    }
    else
    {
        return false;
    }
}

string file_permissionToString(mode_t fp)
{
    string permissions = "";

    if (S_ISDIR(fp))
        permissions += "d";
    if (S_ISREG(fp))
        permissions += "-";

    if (fp & S_IRUSR)
        permissions += "r";
    else
        permissions += "-";

    if (fp & S_IWUSR)
        permissions += "w";
    else
        permissions += "-";

    if (fp & S_IXUSR)
        permissions += "x";
    else
        permissions += "-";

    if (fp & S_IRGRP)
        permissions += "r";
    else
        permissions += "-";

    if (fp & S_IWGRP)
        permissions += "w";
    else
        permissions += "-";

    if (fp & S_IXGRP)
        permissions += "x";
    else
        permissions += "-";

    if (fp & S_IROTH)
        permissions += "r";
    else
        permissions += "-";

    if (fp & S_IWOTH)
        permissions += "w";
    else
        permissions += "-";

    if (fp & S_IXOTH)
        permissions += "x";
    else
        permissions += "-";
    return permissions;
}

// takes as input const char array only
pair<int, string> displayFiles(const char *directory_path, int pointer, int upper, int lower)
{
    vector<array<string, 6>> details;
    bool flag;
    struct dirent *direntp;
    DIR *dirp = opendir(directory_path);

    if (dirp == NULL)
    {
        cout << "Error : " << directory_path << strerror(errno) << endl;
        // maybe some sort of exit
        // return;
    }
    while ((direntp = readdir(dirp)) != NULL)
    {

        string file_name = direntp->d_name;

        string file_path = directory_path;
        file_path += "/";
        file_path += file_name;

        const char *file_path_in_char_array = file_path.c_str();

        struct stat file_stats;
        struct passwd *user;
        struct group *group;

        if (stat(file_path_in_char_array, &file_stats) == -1)
        {
            cout << "Error : " << directory_path << strerror(errno) << endl;
            // return;
        }

        if ((user = getpwuid(file_stats.st_uid)) == NULL)
        {
            cout << "Error : " << directory_path << strerror(errno);
            // return;
        }

        if ((group = getgrgid(file_stats.st_gid)) == NULL)
        {
            cout << "Error : " << directory_path << strerror(errno);
            // return;
        }

        off_t file_size = file_stats.st_size;

        string file_size_string = formatFilesize(file_size);

        string user_name = user->pw_name;

        string group_name = group->gr_name;

        mode_t file_permission = file_stats.st_mode;

        string file_permission_string = file_permissionToString(file_permission);

        time_t modification_time = file_stats.st_mtime;

        struct tm *time_info;
        char modification_time_char[200];

        time_info = localtime(&modification_time);
        strftime(modification_time_char, sizeof(modification_time_char), "%d-%m-%Y %H:%M:%S", time_info);

        string modification_time_string = modification_time_char;
        details.push_back({file_size_string, user_name, group_name, file_permission_string, modification_time_string, file_name});
    }

    sort(details.begin(), details.end(), compareFilenames);
    for (int i = upper; i <= min<int>(lower, details.size() - 1); i++)
    {
        if (i == pointer)
        {
            // cout<<pointer<<" "<<upper<<" "<<lower<<" "<<details.size();
            cout << "->";
        }
        for (int j = 0; j < 6; j++)
        {
            if (j == 0)
            {
                cout << "\t" << setw(15) << left << details[i][j];
            }
            if (j == 1)
            {
                cout << setw(15) << left << details[i][j];
            }
            if (j == 2)
            {
                cout << setw(15) << left << details[i][j];
            }
            if (j == 3)
            {
                cout << setw(15) << left << details[i][j];
            }
            if (j == 4)
            {
                cout << setw(30) << left << details[i][j];
            }
            if (j == 5)
            {
                cout << setw(20) << left << details[i][j];
            }
        }
        cout << endl;
    }

    closedir(dirp);
    std::setvbuf(stdout, NULL, _IONBF, 0);
    cout << "\033[9999;1H";

    if (!FLAG)
    {
        cout << "Normal mode :: " << directory_path;
    }
    else
    {
        cout << "Command mode :: " << directory_path << " ::" << s << " ";
    }

    string pointing_to = details[pointer][5];

    return make_pair(details.size(), pointing_to);
}

struct termios orig_termios;

void disableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    {
        cout << "Error : " << strerror(errno);
    }
}
void enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    {
        cout << "Error : " << strerror(errno);
    }
    atexit(disableRawMode);

    struct termios raw = orig_termios;

    // raw.c_oflag &= ~(OPOST);
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cflag |= (CS8);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    {
        cout << "Error : " << strerror(errno);
    }
}
void enableRawModeForCommand()
{
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    {
        cout << "Error : " << strerror(errno);
    }
    atexit(disableRawMode);

    struct termios raw = orig_termios;

    // raw.c_oflag &= ~(OPOST);
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_lflag &= ~(ICANON | IEXTEN | ISIG);
    raw.c_cflag |= (CS8);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    {
        cout << "Error : " << strerror(errno);
    }
}
void displayDirectory(const char *directory_path)
{
    getWindowSize();
    //signal(SIGWINCH, sig_handler);
    FLAG = false;
    //getWindowSize();
    int upper = 0;
    int lower = rows - 4;
    clearScreen();
    string filename;
    int pointer = 0;
    pair<int, string> information;
    information = displayFiles(directory_path, pointer, upper, lower);
    while (1)
    {
        pointer = stoi(processKeypressForDisplay(pointer, information.first, information.second, directory_path, upper, lower));
        if (pointer == -2)
        {
            return;
        }

        clearScreen();
        information = displayFiles(directory_path, pointer, upper, lower);
    }
}
bool search(string to_search, const char *directory_path)
{

    DIR *dirp = opendir(directory_path);
    if (dirp == NULL)
    {
        return 0; // Print error and continue;
    }
    struct dirent *direntp;

    while ((direntp = readdir(dirp)) != NULL)
    {
        string name = direntp->d_name;
        if (to_search == name)
        {
            closedir(dirp);
            return true;
        }
        // cout << direntp->d_type << "  " << direntp->d_name << endl;

        if (direntp->d_type == DT_DIR && name != "." && name != "..")
        {
            string path = directory_path;
            path += "/";
            path += name;
            const char *path_chars = path.c_str();
            if (search(to_search, path_chars))
            {
                closedir(dirp);
                return true;
            }
        }
    }

    closedir(dirp);
    return false;
}

int mydelete(const char *directory_path)
{
    string directory_path_string = directory_path;
    DIR *dirp = opendir(directory_path);
    if (dirp == NULL)
    {
        cout << "Error: Directory cant be opened";
        return 0; // Print error and continue;
    }
    struct dirent *direntp;

    while ((direntp = readdir(dirp)) != NULL)
    {
        string name = direntp->d_name;
        // skip entries "." and ".."
        if (name == "." || name == "..")
        {
            continue;
        }

        string total_path = directory_path_string + "/" + name;
        const char *total_path_chars = total_path.c_str();

        struct stat file_stats;

        stat(total_path_chars, &file_stats);
        mode_t file_permission = file_stats.st_mode;

        if (S_ISDIR(file_permission))
        {
            mydelete(total_path_chars);
            continue; // abhiyeh entry process ho gaya..moveto next record in directory
        }

        if (S_ISREG(file_permission))
        {
            if (unlink(total_path_chars) == 0)
                cout << "File deleted";
            else
            {
                cout << "Error :Could not delete file";
                return 0;
            }
        }
    }

    if (rmdir(directory_path) == 0)
        cout << "Removed a directory";
    else
    {
        cout << "Error: Could not delete directory";
        return 0;
    }

    closedir(dirp);
    return 1;
}

int copyDirectory(string src, string dest)
{
    const char *src_char = src.c_str();
    const char *dest_char = dest.c_str();
    mode_t m = (S_IRUSR | S_IWUSR | S_IXUSR);

    if (mkdir(dest_char, m) == -1)
    {
        cout << "Error :Could not create directory in the present directory";
        return 0;
    }
    else
    {
        cout << "Directory create successfully";
    }

    // mkdir(dest_char, m);

    cout << "Made directory: " << dest_char << endl;

    DIR *dirp = opendir(src_char);
    if (dirp == NULL)
    {
        cout << "Error: Directory cant be opened";
        return 0; // Print error and continue;
    }

    struct dirent *direntp;
    while ((direntp = readdir(dirp)) != NULL)
    {
        string name = direntp->d_name;
        // skip entries "." and ".."
        if (name == "." || name == "..")
        {
            continue;
        }

        string total_path_src = src + "/" + name;
        string total_path_dest = dest + "/" + name;

        const char *total_path_src_char = total_path_src.c_str();
        const char *total_path_dest_char = total_path_dest.c_str();

        struct stat file_stats;
        stat(total_path_src_char, &file_stats);
        mode_t file_permission = file_stats.st_mode;

        if (S_ISDIR(file_permission))
        {
            cout << "Calling recursively : " << total_path_src << " and " << total_path_dest << endl;
            copyDirectory(total_path_src, total_path_dest);
            continue; // abhiyeh entry process ho gaya..moveto next record in directory
        }

        if (S_ISREG(file_permission))
        {
            char buf;
            int s, d;

            s = open(total_path_src_char, O_RDWR);

            if (s == -1)
            {
                printf("Error opening first_file\n");
                close(s);
                return 0;
            }

            d = open(total_path_dest_char,
                     O_WRONLY | O_CREAT,
                     S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

            while (read(s, &buf, 1))
            {
                write(d, &buf, 1);
            }

            printf("Successful copy\n");

            close(s);
            close(d);
        }
    }

    closedir(dirp);
    return 1;
}

string commandMode(const char *directory_path)
{
    clearScreen();
    FLAG = true;
    string directory_path_string = directory_path;

    displayFiles(directory_path, 0, 0, rows - 4);
    // cout<<"IN command mode : ";
    // cout<<directory_path;
    while (1)
    {

        clearScreen();
        displayFiles(directory_path, 0, 0, rows - 4);

        enableRawModeForCommand();

        char k;
        k = getchar();

        if (k == 27)
        {
            clearScreen();
            disableRawMode();
            path_returned_from_normal_mode = directory_path_string;
            return "esc";
        }
        else if (k == '\r')
        {
            clearScreen();
            disableRawMode();
            continue;
        }
        else if (k == 127)
        {
            clearScreen();
            disableRawMode();
            continue;
        }

        disableRawMode();
        s = "";
        string command_as_string;

        getline(cin, command_as_string);
        // cout<<"command as string: "<<command_as_string;
        if (!(k >= 'a' && k <= 'z'))
        {
            continue;
        }
        string temp(1, k);
        temp += command_as_string;
        command_as_string = temp;

        if (command_as_string.empty())
        {
            continue;
        }

        // if user wants to quit
        if (command_as_string == "quit")
        {
            clearScreen();
            return "quit";
        }

        if (command_as_string == "esc")
        {
            return "esc";
        }

        vector<string> command_in_vector;
        string line;
        istringstream ss(command_as_string);
        while (ss >> line)
        {
            command_in_vector.push_back(line);
        }

        int size_of_command = command_in_vector.size();

        string command_type = command_in_vector[0];

        string source_string;

        const char *source;

        string dest_string = command_in_vector[size_of_command - 1];
        const char *dest;

        if (command_type == "rename")
        {
            // do a if size of vec is !=3 then incorrect command
            if (size_of_command != 3)
            {
                cout << "Error invalid argument count";
                continue;
            }
            source_string = command_in_vector[1];
            // that is source is just a file name
            if (source_string.find('/') == string::npos)
            {
                source_string = directory_path_string + "/" + source_string;
            }
            else if (source_string[0] == '/' /*|| source_string[0] == '~'*/)
            {
                // its absolut path
            }
            else
            {
                source_string = directory_path_string + "/" + source_string;
            }
            source = source_string.c_str();

            int index = source_string.rfind('/');

            dest_string = source_string.substr(0, index + 1);
            dest_string += command_in_vector[size_of_command - 1];
            // cout<<dest_string;

            dest = dest_string.c_str();

            if (rename(source, dest) != 0){
                perror("Error : Incorrect arguments for rename command");
                continue;
            }
            else
                cout << "File renamed successfully";
        }

        else if (command_type == "move")
        {

            for (int i = 1; i < size_of_command - 1; i++)
            {
                dest_string = command_in_vector[size_of_command - 1];
                if (dest_string[0] == '/' /*|| dest_string[0] == '~'*/)
                {
                    // its absolute path, no change needed in parameter
                }
                else
                {
                    dest_string = directory_path_string + "/" + dest_string;
                }
                // cout<<dest_string<<endl;
                source_string = command_in_vector[i];
                if (source_string[0] == '/' /*|| source_string[0] == '~'*/)
                {
                    // its absolute path, no change needed in parameter
                }
                else
                {
                    source_string = directory_path_string + "/" + source_string;
                }

                int index = source_string.rfind('/');

                // cout<<source_string.substr(index);
                // cout<<dest_string<<endl;
                dest_string += source_string.substr(index);
                // cout<<dest_string<<endl;
                dest = dest_string.c_str();
                source = source_string.c_str();
                cout << source << "        " << dest << endl;

                if (rename(source, dest) != 0){
                    perror("Error : Incorrect arguments for move");
                    continue;
                }
                else
                    cout << "File moved successfully";
            }
        }
        else if (command_type == "create_file")
        {
            if (dest_string[0] == '/' /*|| dest_string[0] == '~'*/)
            {
                // its absolute path, no change needed in parameter
            }
            else
            {
                dest_string = directory_path_string + "/" + dest_string;
            }
            string final_path = dest_string + "/" + command_in_vector[1];
            const char *final_path_chars = final_path.c_str();
            mode_t m = (S_IRUSR | S_IWUSR | S_IXUSR);
            cout << final_path_chars << endl;
            if (creat(final_path_chars, m) == -1)
            {
                cout << "Error :Could not create file in the directory";
                continue;
            }
            else
            {
                cout << "File create successfully";
            }
        }
        else if (command_type == "create_dir")
        {
            if (dest_string[0] == '/' /*|| dest_string[0] == '~'*/)
            {
                // its absolute path, no change needed in parameter
            }
            else
            {
                dest_string = directory_path_string + "/" + dest_string;
            }
            string final_path = dest_string + "/" + command_in_vector[1];
            const char *final_path_chars = final_path.c_str();
            mode_t m = (S_IRUSR | S_IWUSR | S_IXUSR);
            // cout<<final_path_chars<<endl;
            if (mkdir(final_path_chars, m) == -1)
            {
                cout << "Error :Could not create directory in the present directory";
                continue;
            }
            else
            {
                cout << "Directory create successfully";
            }
        }
        else if (command_type == "goto")
        {

            if (dest_string[0] == '/' /*|| dest_string[0] == '~'*/)
            {

                // its absolute path, no change needed in parameter
            }

            else if (dest_string == ".")
            {
                dest_string = directory_path_string;
            }
            else if (dest_string == "..")
            {

                int pos = directory_path_string.rfind('/');
                directory_path_string = directory_path_string.substr(0, pos);
                if (directory_path_string.size() == 0)
                {
                    directory_path_string = "/";
                }
                dest_string = directory_path_string;
            }
            else if(dest_string[0]=='~'){
                if(dest_string.size()==1){
                    dest_string=getHome();
                }
                else if(dest_string.size()>1){
                    dest_string=getHome()+dest_string.substr(1);
                }
            }
            else
            {
                dest_string = directory_path_string + "/" + dest_string;
            }
            dest = dest_string.c_str();
            if (chdir(dest) == -1)
            {
                cout << "Error : No such location";
                continue;
            }
            string returned_value = commandMode(dest);
            return returned_value;
        }
        else if (command_type == "copy")
        {

            for (int i = 1; i < size_of_command - 1; i++)
            {
                dest_string = command_in_vector[size_of_command - 1];
                if (dest_string[0] == '/' /*|| dest_string[0] == '~'*/)
                {
                    // its absolute path, no change needed in parameter
                }
                else
                {
                    dest_string = directory_path_string + "/" + dest_string;
                }
                source_string = command_in_vector[i];
                if (source_string[0] == '/' /*|| source_string[0] == '~'*/)
                {
                    // its absolute path, no change needed in parameter
                }
                else
                {
                    source_string = directory_path_string + "/" + source_string;
                }

                int index = source_string.rfind('/');

                // cout<<source_string.substr(index);
                // cout<<dest_string<<endl;
                dest_string += source_string.substr(index);
                // cout<<dest_string<<endl;
                dest = dest_string.c_str();
                source = source_string.c_str();
                // cout << source << "\t" << dest;
                struct stat file_stats;
                if (stat(source, &file_stats) == -1)
                {
                    cout << "Error : " << source << strerror(errno) << endl;
                    continue;
                }
                mode_t file_permission = file_stats.st_mode;
                if (S_ISDIR(file_permission))
                {
                    // cout<<source<<"\t"<<dest<<endl;
                    if (!copyDirectory(source, dest))
                    {
                        continue;
                    }
                }

                if (S_ISREG(file_permission))
                {
                    // cout<<"hi";
                    char buf;
                    int s, d;

                    s = open(source, O_RDONLY);

                    if (s == -1)
                    {
                        printf("Error opening first_file\n");
                        close(s);
                        continue;
                    }

                    d = open(dest,
                             O_WRONLY | O_CREAT,
                             S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

                    while (read(s, &buf, 1))
                    {
                        write(d, &buf, 1);
                    }

                    printf("Successful copy");

                    close(s);
                    close(d);
                }
            }
        }
        else if (command_type == "search")
        {
            if (size_of_command != 2)
            {
                cout << "Error: Incorrect arguments for search";
                continue;
            }
            string to_search = command_in_vector[1];
            // cout<<to_search<<"   "<<directory_path;
            search(to_search, directory_path) ? s = "true" : s = "false";
        }
        else if (command_type == "delete")
        {
            if (size_of_command != 2)
            {
                cout << "Error: Incorrect arguments for delete";
                continue;
            }
            string to_delete = command_in_vector[1];

            string total_path;
            if (to_delete[0] == '/' /*|| dest_string[0] == '~'*/)
            {
                total_path = to_delete;
            }
            else
            {
                total_path = directory_path_string + "/" + to_delete;
            }

            const char *total_path_chars = total_path.c_str();
            cout << total_path_chars;

            struct stat file_stats;
            if (stat(total_path_chars, &file_stats) == -1)
            {
                cout << "Error : " << source << strerror(errno) << endl;
                continue;
            }
            mode_t file_permission = file_stats.st_mode;

            if (S_ISDIR(file_permission))
            {
                if (!mydelete(total_path_chars))
                {
                    continue;
                }
                else
                {
                    cout << "Deleted successfully";
                }
            }

            if (S_ISREG(file_permission))
            {
                if (unlink(total_path_chars) == 0)
                {
                    cout << "File deleted";
                }
                else
                {
                    cout << "Error :Could not delete file";
                    continue;
                }
            }

            // cout<<to_delete<<"   "<<directory_path<<endl;
        }
        
        if (command_as_string == "quit")
        {
            return "quit";
        }
    }
}

// static void sig_handler(int sig)
// {
//     if (SIGWINCH == sig)
//     {
//         struct winsize winsz;

//         ioctl(0, TIOCGWINSZ, &winsz);

//         rows = winsz.ws_row; //, winsz.ws_col);
//     }
// }

int main(void)
{
    //signal(SIGWINCH, sig_handler);
    
    clearScreen();

    getWindowSize();

    const char *home = getHome().c_str();
    const char *begin = home;
    char mode = 'n';
    while (1)
    {
        switch (mode)
        {
        case 'n':
            clearScreen();
            enableRawMode();
            displayDirectory(begin);
            mode = 'c';
            disableRawMode();
            break;
            // if command mode returns an escape character then set mode ='n' naitoh if
            //  it returns "quit" then exit(0) kar do
        case 'c':
            disableRawMode();
            string outcome = commandMode(path_returned_from_normal_mode.c_str());
            if (outcome == "quit")
            {
                clearScreen();
                exit(0);
            }
            else if (outcome == "esc")
            {
                begin = path_returned_from_normal_mode.c_str();
                mode = 'n';
            }
            break;
        } // if command mode returns an escape character then set mode ='n' naitoh if
          //  it returns "quit" then exit(0) kar do
    }

    return 0;
}
