#include "com_kanke_xmpp_Client.h"

#include "talk/base/cryptstring.h"
#include "talk/base/logging.h"
#include "talk/xmpp/xmppclientsettings.h"
#include "examples/echo/echothread.h"

#undef NDEBUG
#ifdef NDEBUG
#define LOGD(...) do { } while(0)
#define LOGE(...) do { } while(0)
#else
#include <android/log.h>
#define  LOG_TAG    "libgxmpp"
#define  LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#endif

#if 0
// 禁止使用拷贝构造函数和 operator= 赋值操作的宏
// 应该类的 private: 中使用
#define DISALLOW_COPY_AND_ASSIGN(TypeName)      \
  TypeName(const TypeName&);                    \
  void operator=(const TypeName&)
#endif

static const int kDefaultXmppPort = 5222;
static talk_base::scoped_ptr<echo::EchoThread> kThread;
class ClientHandler;
static ClientHandler *kClientHandler = NULL;
static JavaVM *kJavaVM = NULL;

class ClientHandler : public echo::XmppHandler
{
 public:
  ClientHandler(jobject jhandler)
      : handler_(jhandler)
  {
    // nil
  };
  void Init() 
  {
    LOGD(__PRETTY_FUNCTION__);
    JNIEnv *env = NULL;
    jint status = JNI_ERR;
    jboolean attached = false;
    status = kJavaVM->GetEnv(reinterpret_cast<void**>(&env),
                             JNI_VERSION_1_6);
    if (JNI_OK != status) {
      if (JNI_EDETACHED != status)
      {
        LOGE("GetEnv error: %d", status);
        return;
      }
      status = kJavaVM->AttachCurrentThread(&env, NULL);
      if (JNI_OK != status)
      {
        LOGE("AttachCurrentThread error: %d", status);
        return;
      }
      attached = true;
    }
    jobject handler = NULL;
    handler = env->NewGlobalRef(handler_);
    if (NULL == handler)
    {
      LOGE("%s NewGlobalRef", __PRETTY_FUNCTION__);
      return;
    }
    handler_ = handler;
    if (attached)
    {
      status = kJavaVM->DetachCurrentThread();
      if (JNI_OK != status) {
        LOGE("DetachCurrentThread() error: %d", status);
        return;
      }
    }
  }
  virtual ~ClientHandler() 
  {
    LOGD(__PRETTY_FUNCTION__);
    JNIEnv *env = NULL;
    jint status = JNI_ERR;
    jboolean attached = false;
    status = kJavaVM->GetEnv(reinterpret_cast<void**>(&env),
                             JNI_VERSION_1_6);
    if (JNI_OK != status) {
      if (JNI_EDETACHED != status)
      {
        LOGE("GetEnv error: %d", status);
        return;
      }
      status = kJavaVM->AttachCurrentThread(&env, NULL);
      if (JNI_OK != status)
      {
        LOGE("AttachCurrentThread error: %d", status);
        return;
      }
      attached = true;
    }
   
    env->DeleteGlobalRef(handler_);

    if (attached)
    {
      status = kJavaVM->DetachCurrentThread();
      if (JNI_OK != status) {
        LOGE("DetachCurrentThread() error: %d", status);
        return;
      }
    }
  };
  virtual std::string OnXmppMessage(const buzz::Jid& from,
                                    const buzz::Jid& to,
                                    const std::string& message) {
    LOGD(__PRETTY_FUNCTION__);
    JNIEnv *env = NULL;
    jint status = JNI_ERR;
    jboolean attached = false;
    status = kJavaVM->GetEnv(reinterpret_cast<void**>(&env),
                             JNI_VERSION_1_6);
    if (JNI_OK != status) {
      if (JNI_EDETACHED != status)
      {
        LOGE("GetEnv error: %d", status);
        return std::string("GetEnv error");
      }
      status = kJavaVM->AttachCurrentThread(&env, NULL);
      if (JNI_OK != status)
      {
        LOGE("AttachCurrentThread error: %d", status);
        return std::string("AttachCurrentThread error");
      }
      attached = true;
    }
    jclass clazz = NULL;
    clazz = env->GetObjectClass(handler_);
    if (NULL == clazz)
    {
      LOGD("FindClass error");
      return std::string("FindClass error");
    }
    jmethodID methodID = NULL;
    methodID = env->GetMethodID(clazz,
                                "HandleMessage",
                                "(Ljava/lang/String;)Ljava/lang/String;");
    if (NULL == methodID) {
      LOGE("GetMethodID error!");
      return std::string("GetMethodID error");
    }
    //LOGD("GetMethodID done");
    const char *request = message.c_str();
    
    jstring jrequest = env->NewStringUTF(request);
    
    jobject handler = NULL;
    handler = env->NewGlobalRef(handler_);
    if (NULL == handler)
    {
      LOGE("NewGlobalRef error");
      return std::string("NewGlobalRef error");
    }
    //LOGD("CallObjectMethod Start");
    jobject obj = env->CallObjectMethod(handler, methodID, jrequest);

    jstring jresponse = static_cast<jstring>(obj);
    const char * response = env->GetStringUTFChars(jresponse, NULL);
    std::string Response(response);
    env->ReleaseStringUTFChars(jresponse, response);

    env->DeleteGlobalRef(handler);
    env->DeleteLocalRef(jrequest);
    if (attached)
    {
      status = kJavaVM->DetachCurrentThread();
      if (JNI_OK != status) {
        LOGE("DetachCurrentThread error: %d", status);
        return std::string("DetachCurrentThread error");
      }
    }

    return Response;
  }
  
  virtual void OnXmppOpen() {
    LOGD(__PRETTY_FUNCTION__);
  }
  
  virtual void OnXmppClosed(int error) {
    LOGD(__PRETTY_FUNCTION__);
  }
 private:
  jobject handler_;
};
// Client

/*
 * Class:     com_kanke_xmpp_Client
 * Method:    init
 * Signature: (Lcom/kanke/xmpp/ClientHandler;)V
 */
JNIEXPORT void JNICALL Java_com_kanke_xmpp_Client_init
(JNIEnv *, jobject, jobject jhandler)
{
  LOGD(__PRETTY_FUNCTION__);

  kClientHandler = new ClientHandler(jhandler);
  kClientHandler->Init();
  kClientHandler->SetResponse(false);
  // talk_base::LogMessage::LogToDebug(talk_base::LS_SENSITIVE);
  // Start xmpp on a different kThread
  kThread.reset(new echo::EchoThread);
  kThread->RegisterXmppHandler(kClientHandler);
  kThread->Start();
}
/*
 * Class:     com_kanke_xmpp_Client
 * Method:    finit
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_kanke_xmpp_Client_finit
(JNIEnv *, jobject)
{
  LOGD(__PRETTY_FUNCTION__);
  kThread.reset();
  delete kClientHandler;
  kClientHandler = NULL;
}
/*
 * Class:     com_kanke_xmpp_Client
 * Method:    Login
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_kanke_xmpp_Client_Login 
(JNIEnv *env, jobject, jstring jjid, jstring jpassword)
{
  LOGD(__PRETTY_FUNCTION__);
  
  const char *jid = env->GetStringUTFChars(jjid, NULL);
  const char *password = env->GetStringUTFChars(jpassword, NULL);

  buzz::Jid Jid(jid);
  if(!Jid.IsValid()) {
    LOGE("bad jid: %s", jid);
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
 * Class:     com_kanke_xmpp_Client
 * Method:    Logout
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_kanke_xmpp_Client_Logout
(JNIEnv *, jobject)
{
  LOGD(__PRETTY_FUNCTION__);
  kThread->Disconnect();
  kThread->Stop();
}
/*
 * Class:     com_kanke_xmpp_Client
 * Method:    SendMessage
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_kanke_xmpp_Client_SendMessage 
(JNIEnv *env, jobject thiz, jstring jto, jstring jmessage)
{
  LOGD(__PRETTY_FUNCTION__);
  const char *to = env->GetStringUTFChars(jto, NULL);
  const char *message = env->GetStringUTFChars(jmessage, NULL);

  jint status = -1;

  buzz::Jid To(to);
  if(!To.IsValid()) {
    LOGE("bad jid: %s", to);
    return status;
  }

  std::string Message(message);

  status = kThread->Send(To, Message);

  env->ReleaseStringUTFChars(jto, to);
  env->ReleaseStringUTFChars(jmessage, message);

  return status;
}

// jni part
// Client
static const char * Client = "com/kanke/xmpp/Client";
static JNINativeMethod ClientMethods[] = {
  {"init",
   "(Lcom/kanke/xmpp/ClientHandler;)V",
   (void *)Java_com_kanke_xmpp_Client_init},
  {"finit",
   "()V",
   (void *)Java_com_kanke_xmpp_Client_finit},
  {"Login",
   "(Ljava/lang/String;Ljava/lang/String;)V",
   (void *)Java_com_kanke_xmpp_Client_Login},
  {"SendMessage",
   "(Ljava/lang/String;Ljava/lang/String;)I",
   (void *)Java_com_kanke_xmpp_Client_SendMessage},
  {"Logout",
   "()V",
   (void *)Java_com_kanke_xmpp_Client_Logout}
};

static int registerNativeMethods(JNIEnv* env, const char* className,
                                 JNINativeMethod* methods, int numMethods) {
  jclass clazz;
  clazz = env->FindClass(className);
  if (clazz == NULL) {
    return JNI_FALSE;
  }
  if (env->RegisterNatives(clazz, methods, numMethods) < 0) {
    return JNI_FALSE;
  }

  return JNI_TRUE;
}

static int registerNatives(JNIEnv* env) {
  if (!registerNativeMethods(env, Client, ClientMethods,
                             sizeof(ClientMethods)/sizeof(ClientMethods[0]))) {
    return JNI_FALSE;
  }
  return JNI_TRUE;
}

// This function will be call when the library first be loaded
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  JNIEnv* env = NULL;
  kJavaVM = vm;
  //LOGD("check JNI version");
  if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6)
      != JNI_OK) {
    LOGE("not support JNI 1.6");
    return JNI_ERR;
  }
  //LOGD("register native method");
  if (registerNatives(env) != JNI_TRUE) {
    LOGE("register navtive method error");
    return JNI_ERR;
  }
  //LOGD("JNI_OnLoad done");
  return JNI_VERSION_1_6;
}
