#include <fstream>
#include <iostream>
#include <stdint.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/sdp.h>

std::ofstream fout("output.txt");

int main(int argc, char **argv)
{
    std::cout << "Test 1" << std::endl;
    fout << "marker 0" << std::endl;

    struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
    char source[18] = "B8:27:EB:57:CC:EE"; //"50:F0:D3:40:42:55";
    bdaddr_t tmp1 = {};
    std::cout << "Test 2" << std::endl;
    str2ba( source, &tmp1 );
    rem_addr.rc_family = AF_BLUETOOTH;
    rem_addr.rc_bdaddr = tmp1;
    rem_addr.rc_channel = 1;

    char buf[1024] = { 0 };
    int s, client, bytes_read;
    unsigned int opt = sizeof(rem_addr);

    //allocate socket
    std::cout << "Test 3" << std::endl;
    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if( s == -1 ) fout << "socket error" << std::endl;
    std::cout << "Test 4" << std::endl;

    //bind socket to port 1 of the first available adapter

    bdaddr_t tmp2 = {0,0,0,0,0,0};

    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = tmp2;
    loc_addr.rc_channel = 1;

    std::cout << "Test 5" << std::endl;
    int error_check = bind(s, (struct sockaddr* )&loc_addr, sizeof(loc_addr));
    if( error_check == -1 ) fout << "binding error" << std::endl;
    std::cout << "Test 6" << std::endl;

    //put socket into listening mode

    error_check = 0;
    error_check = listen(s, 1);
    std::cout << "Test 7" << std::endl;
    if( error_check == -1 ) fout << "listening error" << std::endl;
    std::cout << "Test 8" << std::endl;

    //accept one connection
    client = accept(s, (struct sockaddr *)&rem_addr, &opt);
    std::cout << "Test 9" << std::endl;


    ba2str( &rem_addr.rc_bdaddr, buf );
    fprintf( stderr, "accepter connection from %s\n", buf);
    memset( buf, 0, sizeof(buf));
    std::cout << "Test 10" << std::endl;

    //read data from the client
    bytes_read = recv(client, buf, sizeof(buf), 0);
    std::cout << "Test 11" << std::endl;
    if( bytes_read > 0 )
    {
        fout << buf[0];
    }
    std::cout << "Test 12" << std::endl;

    //close connection
    //close(client);
    //close(s);
    return 0;
}