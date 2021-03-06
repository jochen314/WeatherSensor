/*
 * Utils.cpp
 *
 *  Created on: 06.05.2018
 *      Author: jochen
 */

#include "Utils.h"

Utils::Utils() {
	// TODO Auto-generated constructor stub

}

Utils::~Utils() {
	// TODO Auto-generated destructor stub
}


int Utils::mkpath(std::string s, mode_t mode) {
    size_t pre=0,pos;
    std::string dir;
    int mdret = 0;

//    std::cout << "# checking " << s << std::endl;

    if(s[s.size()-1]!='/'){
        // force trailing / so we can handle everything in loop
        s+='/';
    }

    while((pos=s.find_first_of('/',pre))!=std::string::npos){
        dir=s.substr(0,pos++);
        pre=pos;
        if(dir.size()==0) continue; // if leading / first time is 0 length

//        std::cout << "# creating " << dir << " " ;

        if((mdret=mkdir(dir.c_str(),mode)) && errno!=EEXIST){
//            std::cout << mdret << " " << errno << std::endl ;
            return mdret;
        }
//        std::cout << mdret << " " << errno << std::endl ;
    }
    return mdret;
}
