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

#include <stdio.h>
#include <string.h>
#include "talk/base/cryptstring.h"
#include "talk/base/logging.h"
#include "talk/xmpp/xmppclientsettings.h"
#include "echothread.h"

const int kDefaultXmppPort = 5222;

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("%s jid password\n", argv[0]);
    return -1;
  }

  buzz::Jid jid(argv[1]);
  if(!jid.IsValid()) {
    printf("%s bad jid\n", argv[1]);
    return -1;
  }

  talk_base::InsecureCryptStringImpl password;
  password.password() = argv[2];

  talk_base::LogMessage::LogToDebug(talk_base::LS_SENSITIVE);

  bool reconnect = true;
  while (reconnect) {

    // Start xmpp on a different thread
    echo::Echo thread;
    //buzz::XmppThread thread;
    thread.Start();

    // Create client settings
    buzz::XmppClientSettings xcs;
    xcs.set_user(jid.node());
    xcs.set_pass(talk_base::CryptString(password));
    xcs.set_host(jid.domain());
    xcs.set_resource("chat");
    //xcs.set_use_tls(buzz::TLS_DISABLED);
    xcs.set_server(talk_base::SocketAddress(jid.domain(), kDefaultXmppPort));

    thread.Login(xcs);

    thread.Send(buzz::Jid("li@xmpp.ikanke.cn"), "hello");
    // Use main thread for console input
    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, stdin)) {
      printf("you input: %s\n", line);
      if (!strncmp("quit", line, 4) || !strncmp("q", line, 1)) {
        printf("%s\n", line);
        reconnect = false;
        break;
      }
      if (!strncmp("continue", line, 7) || 
          !strncmp("c", line, 1))  {
        break;
      }
    }
    thread.Disconnect();
    thread.Stop();
  }

  return 0;
}
