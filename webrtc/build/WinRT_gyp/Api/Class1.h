#pragma once

#include <collection.h>

using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace Platform::Collections;

namespace webrtc_winrt_api
{
  // Custom struct
  public value struct PlayerData
  {
    Platform::String^ Name;
    int Number;
    double ScoringAverage;
  };

  public ref class Player sealed
  {
  private:
    PlayerData m_player;
  public:
    property PlayerData PlayerStats
    {
      PlayerData get()
      {
        return m_player;
      }
      void set(PlayerData data)
      {
        m_player = data;
      }
    }

    static IVector<Player^>^ MakePlayers(int count)
    {
      auto ret = ref new Vector<Player^>();
      for (int i = 0; i < count; ++i)
      {
        auto player = ref new Player();
        player->PlayerStats = { L"Player", i, i + 0.5 };
        ret->Append(player);
      }
      return ret;
    }
  };
}
