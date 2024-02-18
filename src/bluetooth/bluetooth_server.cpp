#include "bluetooth_server.h"

#include "core/core.h"
#include "Debug.h"

#include <stdlib.h>

extern "C"
{
#include "btferret/btlib.h"
}

int le_callback(int clientnode, int operation, int cticn, void (*onMessage)(unsigned char *data));

void startBluetoothServer(void (*onMessage)(unsigned char *data))
{
  int index;
  unsigned char buf[244], uuid[2];

  if (init_blue("../devices.txt") == 0)
    return;
  // write 56 to Info (index 5 in devices.txt)
  // or find index from UUID
  uuid[0] = 0xCD;
  uuid[1] = 0xEF;
  index = find_ctic_index(localnode(), UUID_2, uuid); // should be 5

  buf[0] = 0x56;
  write_ctic(localnode(), 5, buf, 0); // local device is allowed to write to its own
                                      // characteristic Info
                                      // Size is known from devices.txt, so last
                                      // parameter (count) can be 0

  // write 12 34 to Control (index 4)
  buf[0] = 0x12;
  buf[1] = 0x34;
  write_ctic(localnode(), 4, buf, 0);

  keys_to_callback(KEY_ON, 0); // OPTIONAL - key presses are sent to le_callback
                               // with operation=LE_KEYPRESS and cticn=key code
                               // The key that stops the server changes from x to ESC
  le_server(le_callback, 100, onMessage);
  // Become an LE server and wait for clients to connect.
  // when a client performs an operation such as connect, or
  // write a characteristic, call the function le_callback()
  // Call LE_TIMER in le_callback every 100 deci-seconds (10 seconds)
  close_all();
  return;
}

int le_callback(int clientnode, int operation, int cticn, void (*onMessage)(unsigned char *data))
{
  unsigned char buf[244];

  if (operation == LE_CONNECT)
  {
    // clientnode has just connected
    DEBUG("Client %d has connected\n", clientnode);
    CORE.isBluetoothConnected = true;
  }
  else if (operation == LE_READ)
  {
    // clientnode has just read local characteristic cticn
    DEBUG("Client %d has read characteristic %d\n", clientnode, cticn);
  }
  else if (operation == LE_WRITE)
  {
    // clientnode has just written local characteristic cticn
    read_ctic(localnode(), cticn, buf, sizeof(buf)); // read characteristic to buf
    // DEBUG("Client %d has written characteristic %d with data %d\n", clientnode, cticn, sizeof(buf));
    onMessage(buf);
  }
  else if (operation == LE_DISCONNECT)
  {
    // clientnode has just disconnected
    // uncomment next line to stop LE server when client disconnects
    // return(SERVER_EXIT);
    // otherwise LE server will continue and wait for another connection
    // or operation from other clients that are still connected
    DEBUG("Client %d has disconnected\n", clientnode);
    CORE.isBluetoothConnected = false;
  }
  else if (operation == LE_TIMER)
  {
    // The server timer calls here every timerds deci-seconds
    // Data (index 6) is notify capable
    // so if the client has enabled notifications for this characteristic
    // the following write will send the data as a notification to the client
    buf[0] = 0x67;
    write_ctic(localnode(), 6, buf, 0);
  }
  else if (operation == LE_KEYPRESS)
  {
    // Only active if keys_to_callback(KEY_ON,0) has been called before le_server()
    // cticn = key code
    //       = ASCII code of key (e.g. a=97)  OR
    //         btferret custom code for other keys such as Enter, Home,
    //         PgUp. Full list in keys_to_callback() section
  }

  return SERVER_CONTINUE;
}
