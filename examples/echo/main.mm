/*
 * gxmpp
 * Copyright 2013, Henry Lee <henglinli@gmail.com>.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include "talk/xmpp/jid.h"
#include "talk/base/logging.h"

#import "client.h"
#ifndef NDEBUG
#   define DLog(fmt, ...) NSLog((@"%s [Line %d] " fmt), __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__);
#else
#   define DLog(...)
#endif
#define ALog(fmt, ...) NSLog((@"%s [Line %d] " fmt), __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__);
#ifndef NDEBUG
#   define ULog(fmt, ...)  { UIAlertView *alert = [[UIAlertView alloc] initWithTitle:[NSString stringWithFormat:@"%s\n [Line %d] ", __PRETTY_FUNCTION__, __LINE__] message:[NSString stringWithFormat:fmt, ##__VA_ARGS__]  delegate:nil cancelButtonTitle:@"Ok" otherButtonTitles:nil]; [alert show]; }
#else
#   define ULog(...)
#endif
  //
@interface Client : GxmppClient <GxmppClientDelegate>
  // when xmpp messge arrived, it's being called
-(void)didOnXmppMessage:(NSString *)message From:(NSString *)from;
  // as it's name
-(void)didOnXmppOpen;
-(void)didOnXmppClosed:(int)error;
@end
  //
@implementation Client
  // when xmpp messge arrived, it's being called
-(void)didOnXmppMessage:(NSString *)message From:(NSString *)from {
  DLog();
}
  // as it's name
-(void)didOnXmppOpen {
  DLog();
}
-(void)didOnXmppClosed:(int)error {
  DLog();
}

@end

int main (int argc, const char * argv[]) {
    if (argc != 3) {
      std::cout << argv[0] << " jid password [server]" << std::endl;
      return -1;
    }

    buzz::Jid Jid(argv[1]);
    if(!Jid.IsValid()) {
      std::cout << argv[1] << " bad jid" << std::endl;
      return -1;
    }
    talk_base::LogMessage::LogToDebug(talk_base::LS_SENSITIVE);
    
  NSString *jid = [[NSString alloc] initWithUTF8String: argv[1]];
    NSString *password = [[NSString alloc] initWithUTF8String: argv[2]];

  NSString *server = @"";//[[NSString alloc] initWithUTF8String: argv[3]];
  
  @autoreleasepool {
    Client *client = [[Client alloc] initWithJid:jid Password:password Server:server];
    client.delegate = client;
    [client login];
      // Use main thread for console input
    std::string line;
    while (std::getline(std::cin, line)) {
      if ("quit" == line || "q" == line) {
        break;
      }
      if("continue" == line || "c" == line) {
        [client logout];
        [client login];
      }
    }
    [client logout];
  }
  return 0;
}