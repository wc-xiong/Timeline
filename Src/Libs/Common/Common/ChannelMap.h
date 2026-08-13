#pragma once

// 各种声道系统

#define CHANNEL_SYSTEM_NONE 0u

#define CHANNEL_SYSTEM_1 1u // 单声道
#define CHANNEL_SYSTEM_1_NAME "1.0"

#define CHANNEL_SYSTEM_2 2u // 立体声
#define CHANNEL_SYSTEM_2_NAME "2.0"

#define CHANNEL_SYSTEM_2_1 3u // 立体声 + 重低音
#define CHANNEL_SYSTEM_2_1_NAME "2.1"

#define CHANNEL_SYSTEM_4 4u // 4 声道
#define CHANNEL_SYSTEM_4_NAME "4.0"

#define CHANNEL_SYSTEM_4_1 5u // 4 声道
#define CHANNEL_SYSTEM_4_1_NAME "4.1"

#define CHANNEL_SYSTEM_5_1 6u // 5 声道 + 重低音
#define CHANNEL_SYSTEM_5_1_NAME "5.1"

#define CHANNEL_SYSTEM_6_1 7u // 6 声道 + 重低音
#define CHANNEL_SYSTEM_6_1_NAME "6.1"

#define CHANNEL_SYSTEM_7_1 8u // 7 声道 + 重低音
#define CHANNEL_SYSTEM_7_1_NAME "7.1"

#define CHANNEL_SYSTEM_MAX CHANNEL_SYSTEM_7_1

// 单声道

#define CHANNEL_SYSTEM_1_FRONT_CENTER 0u

// 立体声

#define CHANNEL_SYSTEM_2_FRONT_LEFT 0u
#define CHANNEL_SYSTEM_2_FRONT_RIGHT 1u

// 立体声 + 重低音

#define CHANNEL_SYSTEM_2_1_FRONT_LEFT 0u
#define CHANNEL_SYSTEM_2_1_FRONT_RIGHT 1u
#define CHANNEL_SYSTEM_2_1_SW 2u // 重低音

// 4 声道

#define CHANNEL_SYSTEM_4_FRONT_LEFT 0u
#define CHANNEL_SYSTEM_4_FRONT_RIGHT 1u
#define CHANNEL_SYSTEM_4_BACK_LEFT 2u
#define CHANNEL_SYSTEM_4_BACK_RIGHT 3u

// 4.1 声道

#define CHANNEL_SYSTEM_4_1_FRONT_LEFT 0u
#define CHANNEL_SYSTEM_4_1_FRONT_RIGHT 1u
#define CHANNEL_SYSTEM_4_1_FRONT_CENTER 2u
#define CHANNEL_SYSTEM_4_1_BACK_LEFT 3u
#define CHANNEL_SYSTEM_4_1_BACK_RIGHT 4u

// 5 声道 + 重低音

#define CHANNEL_SYSTEM_5_1_FRONT_LEFT 0u
#define CHANNEL_SYSTEM_5_1_FRONT_RIGHT 1u
#define CHANNEL_SYSTEM_5_1_FRONT_CENTER 2u
#define CHANNEL_SYSTEM_5_1_SW 3u // 重低音
#define CHANNEL_SYSTEM_5_1_BACK_LEFT 4u
#define CHANNEL_SYSTEM_5_1_BACK_RIGHT 5u

// 6 声道 + 重低音

#define CHANNEL_SYSTEM_6_1_FRONT_LEFT 0u
#define CHANNEL_SYSTEM_6_1_FRONT_RIGHT 1u
#define CHANNEL_SYSTEM_6_1_FRONT_CENTER 2u
#define CHANNEL_SYSTEM_6_1_SW 3u // 重低音
#define CHANNEL_SYSTEM_6_1_BACK_CENTER 4u
#define CHANNEL_SYSTEM_6_1_BACK_LEFT 5u
#define CHANNEL_SYSTEM_6_1_BACK_RIGHT 6u

// 7 声道 + 重低音

#define CHANNEL_SYSTEM_7_1_FRONT_LEFT 0u
#define CHANNEL_SYSTEM_7_1_FRONT_RIGHT 1u
#define CHANNEL_SYSTEM_7_1_FRONT_CENTER 2u
#define CHANNEL_SYSTEM_7_1_SW 3u // 重低音
#define CHANNEL_SYSTEM_7_1_BACK_LEFT 4u
#define CHANNEL_SYSTEM_7_1_BACK_RIGHT 5u
#define CHANNEL_SYSTEM_7_1_SIDE_LEFT 6u
#define CHANNEL_SYSTEM_7_1_SIDE_RIGHT 7u

struct SChannelMap
{
	// CHANNEL_SYSTEM_XXX，即通道数
	unsigned type;

	// map[OUTPUT-CHANNEL] -> INPUT-CHANNEL
	unsigned map[CHANNEL_SYSTEM_MAX];

	inline SChannelMap()
	{
		memset(this, 0, sizeof(*this));
	}

	// 默认映射
	static inline SChannelMap Default(int channelSystem)
	{
		SChannelMap map;
		map.type = channelSystem < CHANNEL_SYSTEM_NONE ? CHANNEL_SYSTEM_NONE : (channelSystem > CHANNEL_SYSTEM_MAX ? CHANNEL_SYSTEM_MAX : channelSystem);
		for (unsigned i = 0; i < map.type; i++)
		{
			map.map[i] = i;
		}
		return map;
	}
};

