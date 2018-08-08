#include <jni.h>
//#include <android/log.h>
#include <Substrate/CydiaSubstrate.h>
#include "Unity/Quaternion.hpp" // C++ equivalent of Unity C# Quaternion.
#include "Unity/Vector3.hpp" // C++ equivalent of Unity C# Vector3.
#include "Unity/Vector2.hpp" // C++ equivalent of Unity C# Vector2.
#include "Unity/Unity.h" // C++ equivalent of Unity List/Dictionary/Array <- Credits to Shmoo for this.
/*
#define LOG_TAG  "JbroMain"

#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
*/

long baseAddr(char *soname)  // credits to https://github.com/ikoz/AndroidSubstrate_hookingC_examples/blob/master/nativeHook3/jni/nativeHook3.cy.cpp
{
    void *imagehandle = dlopen(soname, RTLD_LOCAL | RTLD_LAZY);
    if (soname == NULL)
        return NULL;
    if (imagehandle == NULL){
        return NULL;
    }
    uintptr_t * irc = NULL;
    FILE *f = NULL;
    char line[200] = {0};
    char *state = NULL;
    char *tok = NULL;
    char * baseAddr = NULL;
    if ((f = fopen("/proc/self/maps", "r")) == NULL)
        return NULL;
    while (fgets(line, 199, f) != NULL)
    {
        tok = strtok_r(line, "-", &state);
        baseAddr = tok;
        tok = strtok_r(NULL, "\t ", &state);
        tok = strtok_r(NULL, "\t ", &state); // "r-xp" field
        tok = strtok_r(NULL, "\t ", &state); // "0000000" field
        tok = strtok_r(NULL, "\t ", &state); // "01:02" field
        tok = strtok_r(NULL, "\t ", &state); // "133224" field
        tok = strtok_r(NULL, "\t ", &state); // path field

        if (tok != NULL) {
            int i;
            for (i = (int)strlen(tok)-1; i >= 0; --i) {
                if (!(tok[i] == ' ' || tok[i] == '\r' || tok[i] == '\n' || tok[i] == '\t'))
                    break;
                tok[i] = 0;
            }
            {
                size_t toklen = strlen(tok);
                size_t solen = strlen(soname);
                if (toklen > 0) {
                    if (toklen >= solen && strcmp(tok + (toklen - solen), soname) == 0) {
                        fclose(f);
                        return (long)strtoll(baseAddr,NULL,16);
                    }
                }
            }
        }
    }
    fclose(f);
    return NULL;
}

long location; // save lib.so base address so we do not have to recalculate every time causing lag.

long getRealOffset(long offset) // calculate dump.cs address + lib.so base address.
{
    if (location == 0)
    {
        //arm
        location = baseAddr("/data/app/com.package.name-1/lib/arm/libil2cpp.so"); // replace the com.package.name with the package name of the app you are modding.
        if (location == 0)
        {
            //x86
            location = baseAddr("/data/app-lib/com.package.name-1/libil2cpp.so"); // do the same here.
        }
    }
    return location + offset;
}

void orig_WeaponSounds_Update(void* _this);
void hook_WeaponSounds_Update(void* _this){
    if(_this)
    {
        *(bool*) ((uint64_t) _this + 0xDC) = true;//	public bool isShotGun; // 0xDC
    }
    orig_WeaponSounds_Update(_this);
}

// the constructor is ran when the lib is loaded
// \smali\com\unity3d\player\UnityPlayerActivity.smali -> onCreate
__attribute__((constructor))
void libjbro_main() {
    MSHookFunction((void *) getRealOffset(0xC364A8), (void *) &hook_WeaponSounds_Update, (void **) &orig_WeaponSounds_Update); // 	private void Update(); // 0xC364A8
}