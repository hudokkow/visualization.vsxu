#ifdef __GNUC__
#define __cdecl
#endif

#include "kodi/xbmc_vis_types.h"
#include "kodi/xbmc_vis_dll.h"

#include <vsx_version.h>
#include <vsx_platform.h>
#include <vsx_manager.h>
#include <vsx_gl_state.h>


#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <sys/stat.h>


// TODO: configure setting.
// https://github.com/vovoid/vsxu/blob/master/engine_audiovisual/src/vsx_manager.cpp
// https://github.com/vovoid/vsxu/blob/master/engine_audiovisual/src/vsx_statelist.h

vsx_gl_state gl_state;
vsx_manager_abs* manager = NULL;
bool initialized = false;

bool warnGiven = false;

float audio_data[513];
float audio_data_freq[513];

std::vector<std::string> g_presets;

extern "C" ADDON_STATUS ADDON_Create (void* hdl, void* props)
{
  if (!props)
    return ADDON_STATUS_UNKNOWN;

  //VIS_PROPS* visProps = (VIS_PROPS*) props;

  if (!initialized) {
    initialized = true;
    // create a new manager
    manager = manager_factory();
    manager->set_option_preload_all(false);

    manager->init("/usr/share/vsxu", "media_player"); // TODO setting for vis path
    g_presets = manager->get_visual_filenames();
    // strip off dir names - if there are duped presets this will misbehave.
    for (size_t i=0;i<g_presets.size();++i) {
      size_t sit = g_presets[i].rfind('/');
      size_t dit = g_presets[i].rfind('.');
      g_presets[i] = g_presets[i].substr(sit+1,dit-sit-1);
    }
  }

  return ADDON_STATUS_NEED_SETTINGS;
}

extern "C" void Start (int, int, int, const char*)
{
}

extern "C" void AudioData (const float *pAudioData, int iAudioDataLength, float *pFreqData, int iFreqDataLength)
{
  if ((unsigned int)iAudioDataLength > sizeof(audio_data)/sizeof(*audio_data)-1) {
    if (!warnGiven) {
      fprintf(stderr, "Unexpected audio data length received (%d), expect incorrect vis\n", iAudioDataLength);
      warnGiven = true;
    }
    return;
  }

  if ((unsigned int)iFreqDataLength > sizeof(audio_data_freq)/sizeof(*audio_data_freq)-1) {
    if (!warnGiven) {
      fprintf(stderr, "Unexpected freq data length received (%d), expect incorrect vis\n", iAudioDataLength);
      warnGiven = true;
    }
    return;
  }

  memcpy(audio_data, pAudioData, iAudioDataLength*sizeof(float));
  memcpy(audio_data_freq, pFreqData, iFreqDataLength*sizeof(float));
}

extern "C" void Render()
{
/*  for (unsigned long i = 0; i < 512; i++)
    {
    audio_data[i] = (float)(rand()%65535-32768)*(1.0f/32768.0f);
    }
    for (unsigned long i = 0; i < 512; i++)
    {
    audio_data_freq[i] = (float)(rand()%65535)*(1.0f/65535.0f);
    }
    manager->set_sound_freq(&audio_data[0]);
    manager->set_sound_wave(&audio_data_freq[0]);*/

  manager->set_sound_wave(audio_data);
  manager->set_sound_freq(audio_data_freq);

  manager->render();
}

extern "C" void GetInfo (VIS_INFO* pInfo)
{
  pInfo->bWantsFreq = true;
  pInfo->iSyncDelay = 0;
}

extern "C" bool OnAction (long flags, const void *param)
{
  if (flags == VIS_ACTION_LOAD_PRESET && param)
  {
    int index = *((int*)param);
    manager->pick_visual(index);
    return true;
  }
  else if (flags == VIS_ACTION_NEXT_PRESET)
  {
    manager->next_visual();
    return true;
  }
  else if (flags == VIS_ACTION_PREV_PRESET)
  {
    manager->prev_visual();
    return true;
  }
  else if (flags == VIS_ACTION_LOCK_PRESET)
  {
    manager->toggle_randomizer();
    return true;
  }
  return false;
}

extern "C" unsigned int GetPresets (char ***presets)
{
  if (g_presets.size() > 0) {
    *presets = new char*[g_presets.size()];
    for (size_t i=0;i<g_presets.size();++i)
      (*presets)[i] = strdup(g_presets[i].c_str());
  }
  return g_presets.size();
}

class FindSubString {
  public:
    FindSubString(const std::string& b) : m_b(b) {}

    bool operator()(const std::string& a)
    {
      return m_b.find(a) != std::string::npos;
    }

    std::string m_b;
};

extern "C" unsigned GetPreset()
{
  std::string current = manager->get_meta_visual_filename();

  std::vector<std::string>::const_iterator it = std::find_if(g_presets.begin(),
                                                             g_presets.end(),
                                                             FindSubString(current));

  return it-g_presets.begin();
}

extern "C" bool IsLocked()
{
  return false;
}

extern "C" unsigned int GetSubModules (char ***names)
{
  return 0;
}

extern "C" void ADDON_Stop()
{
  return;
}

extern "C" void ADDON_Destroy()
{
  // It is so slow to start up each time, and destroy happens very often, so don't unload
  // Actually, it isn't slow, but calling Render takes very long the first time

  // stop vsxu nicely (unloads textures and frees memory)
  //if (manager) manager->stop();

  // call manager factory to destruct our manager object
  //if (manager) manager_destroy(manager);

  //manager = NULL;
  //initialized = false;

  return;
}

extern "C" bool ADDON_HasSettings()
{
  return false;
}

extern "C" ADDON_STATUS ADDON_GetStatus()
{
  //    return ADDON_STATUS_UNKNOWN;

  return ADDON_STATUS_OK;
}

extern "C" unsigned int ADDON_GetSettings (ADDON_StructSetting ***sSet)
{
  return 0;
}

extern "C" void ADDON_FreeSettings()
{
}

extern "C" ADDON_STATUS ADDON_SetSetting (const char *strSetting, const void* value)
{
  return ADDON_STATUS_OK;
}

extern "C" void ADDON_Announce(const char *flag, const char *sender, const char *message, const void *data)
{
}
