#include "cn_ikanke_Client.h"

#include "talk/base/cryptstring.h"
#include "talk/base/logging.h"
#include "talk/xmpp/xmppclientsettings.h"
#include "examples/echo/echothread.h"

#define DEBUG 0

#if DEBUG
#include <android/log.h>
#  define  D(x...)  __android_log_print(ANDROID_LOG_INFO,"libgxmpp",x)
#else
#  define  D(...)  do {} while (0)
#endif

static const int kDefaultXmppPort = 5222;
static talk_base::scoped_ptr<echo::EchoThread> kThread;
/*
 * Class:     cn_ikanke_Client
 * Method:    init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_cn_ikanke_Client_init
  (JNIEnv *, jobject)
{
	talk_base::LogMessage::LogToDebug(talk_base::LS_SENSITIVE);
	// Start xmpp on a different kThread
	kThread.reset(new echo::EchoThread);
	kThread->Start();
}

/*
 * Class:     cn_ikanke_Client
 * Method:    finit
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_cn_ikanke_Client_finit
  (JNIEnv *, jobject)
{
	kThread.reset();
}

/*
 * Class:     cn_ikanke_Client
 * Method:    Login
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_cn_ikanke_Client_Login
  (JNIEnv *env, jobject thiz, jstring jjid, jstring jpassword)
{
	const char *jid = env->GetStringUTFChars(jjid, NULL);
	const char *password = env->GetStringUTFChars(jpassword, NULL);

	 buzz::Jid Jid(jid);
	 if(!Jid.IsValid()) {
		 D("bad jid: %s", jid);
		 return;
	 }
	talk_base::InsecureCryptStringImpl Password;
	Password.password() = password;

	// Create client settings
	buzz::XmppClientSettings xcs;
	xcs.set_user(Jid.node());
	xcs.set_pass(talk_base::CryptString(Password));
	xcs.set_host(Jid.domain());
	xcs.set_resource("chat");
	xcs.set_use_tls(buzz::TLS_DISABLED);
	xcs.set_server(talk_base::SocketAddress(Jid.domain(), kDefaultXmppPort));

	kThread->Login(xcs);

	env->ReleaseStringUTFChars(jjid, jid);
	env->ReleaseStringUTFChars(jpassword, password);
}

/*
 * Class:     cn_ikanke_Client
 * Method:    Logout
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_cn_ikanke_Client_Logout
  (JNIEnv *, jobject)
{
	kThread->Disconnect();
	kThread->Stop();
}

/*
 * Class:     cn_ikanke_Client
 * Method:    SendMessage
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_cn_ikanke_Client_SendMessage
  (JNIEnv *env, jobject thiz, jstring jto, jstring jmessage)
{
	const char *to = env->GetStringUTFChars(jto, NULL);
	const char *message = env->GetStringUTFChars(jmessage, NULL);

	jint status = -1;

	buzz::Jid To(to);
	if(!To.IsValid()) {
		D("bad jid: %s", to);
		return status;
	}

	std::string Message(message);

	status = kThread->Send(To, Message);

	env->ReleaseStringUTFChars(jto, to);
    env->ReleaseStringUTFChars(jmessage, message);

    return status;
}
