#include <stdio.h>
#include <string>
#include <sstream>

using namespace std;

string to_string(int val){
    stringstream ss; 
    ss << val;
    return ss.str();
}

string charto_string(char val){
    stringstream ss; 
    ss << val;
    return ss.str();
}

// void logger_cout(const char* content) {
// 	if (islog) {
// 		time_t t;
// 		t = time(NULL);
// 		tm* ptr = localtime(&t);
// 		char str[20];
// 		strftime(str, 20, "%Y-%m-%d %H:%M:%S", ptr);
// 		string log(str);
// 		log.append("-DEBUG:");
// 		log.append(content);
// 		cout << log.c_str() << endl;
// 	}
// }