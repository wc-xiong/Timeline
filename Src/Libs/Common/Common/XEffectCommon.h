#pragma once
#include <QString>
#include <QSize>
#include <QColor>
#include <QJsonObject>
/*-------------------------------------------------------------------------- 离屏渲染特效数据------------------------------------------------------*/

// 像素着色器共用, 值不可修改

/**
 * @file XEffectCommon.h
 * @brief 离屏渲染特效公共定义头文件
 *
 * 本文件定义了 GPU 像素着色器特效相关的全部公共数据结构与参数类型，主要包括：
 * 1. EEffectType   — 特效类型枚举，与像素着色器中的特效索引一一对应，枚举值不可随意修改；
 * 2. S* 系列结构体  — 传递给 GPU 常量缓冲区的紧凑参数块（16 字节对齐）；
 * 3. CBXEffect     — 统一的 GPU 特效参数容器，通过 type 字段区分当前生效的特效；
 * 4. SFxFactor 系列 — 面向 UI/业务层的特效参数结构，支持 JSON 序列化及向 CBXEffect 的转换；
 * 5. EMaskType     — 蒙版形状类型枚举；
 * 6. g_effectNodeInfo — 特效节点注册表，供特效树/节点面板使用。
 */

/*-------------------------------------------------------------------------- 离屏渲染特效数据 ------------------------------------------------------*/

enum EEffectType
{
	None,
	// 模糊
	Blur,
	// 锐化
	Sharpen, 
	// 浮雕
	Emboss,
	// 降噪
	Denoise,
	// 边框
	Frame,
	// 羽化
	Feather,
	// 抠像
	Keying,
	// 闪屏
	Blink,
	// RGB 调整
	RGBA, 
	// 抖动
	Shake,
	// 裁剪
	Cut,
	// 滤色
	Filter,
	// 补色
	Complement,
	// 滚动
	Scroll,
	// 马赛克
	Mosaic,
	// 阴影
	Shadow,
	// HSV 调整
	HSV,
	// 扭曲 - 球面化 (鱼眼)
	Spherize,
	// 扭曲 - 漩涡
	Twirl,
	// 镜像 - 万花筒
	Mirror, 
	// 夜视
	Nightvision,
	// 跑马灯
	Marquees, 
	// 蒙版
	Mask, 
	//转场
	Transation_OneTex, 
	Transation_TwoTex,

	Num,
};

/** @brief 模糊特效 GPU 参数块（16 字节对齐，供像素着色器读取） */
__declspec(align(16)) struct SBlur
{
	int sizeIndex;  // 模糊核尺寸索引，由 SBlurFactor 根据 degree 计算得出
	int x;          // 模糊迭代趟数/附加参数，由 SBlurFactor 根据 scale 计算得出
};

/** @brief 浮雕特效 GPU 参数块 */
__declspec(align(16)) struct SEmboss
{
	int dir;        // 光照方向，0~7：从左上角开始顺时针 8 个方向
	float weight;   // 浮雕加深程度/强度
};

/** @brief 锐化特效 GPU 参数块 */
__declspec(align(16)) struct SSharpen
{
	float weight;   // 锐化强度，范围 0~5
};

/** @brief 降噪特效 GPU 参数块 */
__declspec(align(16)) struct SDenoise
{
	int type;       // 降噪算法类型：0=椒盐噪声，1=高斯噪声
	int x;          // 附加参数（保留字段）
	int weight;     // 降噪强度，范围 0~4
};

/** @brief 边框特效 GPU 参数块 */
__declspec(align(16)) struct SFrame
{
	float r, g, b;  // 边框颜色 RGB 分量，范围 0~1
	float size;     // 边框宽度（归一化），由 SFrameFactor 传入时除以 2
};

/** @brief 羽化特效 GPU 参数块 */
__declspec(align(16)) struct SFeather
{
	float size;     // 羽化范围/过渡带宽度，范围 0~1
};

/** @brief 抠像特效 GPU 参数块 */
__declspec(align(16)) struct SKeying
{
	float r, g, b;      // 抠像目标颜色 RGB 分量，范围 0~1
	float threshold;    // 色度匹配阈值，决定抠像灵敏度
	float weight;       // Y（亮度）分量权重，范围 0~1，用于调节亮度对匹配的影响
};

/** @brief 闪屏特效 GPU 参数块 */
__declspec(align(16)) struct SBlink
{
	float r, g, b, a;   // 闪烁覆盖层颜色 RGBA 分量，范围 0~1
};

/** @brief RGBA 通道调整 GPU 参数块 */
__declspec(align(16)) struct SRgba
{
	float r, g, b, a;   // 各通道增益/偏移量，范围 0~1
};

/** @brief 滤色特效 GPU 参数块 */
__declspec(align(16)) struct SFilter
{
	float r, g, b;      // 滤色目标颜色 RGB 分量，范围 0~1
};

/** @brief 马赛克特效 GPU 参数块 */
__declspec(align(16)) struct SMosaic
{
	float x1, y1, x2, y2;   // 马赛克区域左上角 (x1,y1) 与右下角 (x2,y2)，归一化坐标 0~1
	float size;             // 马赛克块大小，范围 0~1，实际传入着色器时除以 20
};

/** @brief 阴影特效 GPU 参数块 */
__declspec(align(16)) struct SShadow
{
	float r, g, b, a;   // 阴影颜色 RGBA 分量，范围 0~1
	float blur;         // 阴影虚化程度，范围 0~0.5，实际传入着色器时除以 2
};

/** @brief HSV 调整 GPU 参数块 */
__declspec(align(16)) struct SHsv
{
	float h, s, v;      // 色相(H)、饱和度(S)、明度(V) 偏移量
};

/** @brief 球面化扭曲 GPU 参数块 */
__declspec(align(16)) struct SSpherize
{
	float x, y;         // 扭曲中心点，归一化坐标 0~1
	float a, b, c;      // 椭球体三个轴向半径（长/宽/高），范围 0~1
	int clearOutside;   // 是否清除椭球外部区域：0=保留，非0=清除（显示透明/背景）
};

/** @brief 漩涡扭曲 GPU 参数块 */
__declspec(align(16)) struct STwirl
{
	float x, y;         // 漩涡中心点，归一化坐标 0~1
	float r;            // 漩涡影响半径，范围 0~1
	float radian;       // 最大旋转弧度（正值=顺时针，负值=逆时针）
	int clearOutside;   // 是否清除半径外部区域：0=保留，非0=清除
};

/** @brief 万花筒/镜像 GPU 参数块 */
__declspec(align(16)) struct SMirror
{
	float x, y;         // 对称中心点，归一化坐标 0~1
	float section;      // 每个镜像扇区的角度大小（弧度）
	float offset;       // 起始扇区的旋转偏移（弧度）
};

/** @brief 夜视特效 GPU 参数块 */
__declspec(align(16)) struct SNightvision
{
	int noise;          // 是否叠加噪点：0=关闭，非0=开启
};

/** @brief 跑马灯 GPU 参数块 */
__declspec(align(16)) struct SMarquess
{
	float l;            // 灯带长度（占周长比例）
	float x, y;         // 灯带宽度参数 (x=水平, y=垂直)
	float d0;           // 灯带起始距离（沿边缘的偏移）
	int dir;            // 旋转方向：0=逆时针，非0=顺时针
}; // 跑马灯

/** @brief 蒙版 GPU 参数块 */
__declspec(align(16)) struct SMask
{
	int reverse;        // 是否反转蒙版：0=正常，非0=反转（显示蒙版外部）
	int bPicture;       // 蒙版来源：0=内置形状，非0=自定义图片蒙版
}; // 蒙版

/** @brief 转场 GPU 参数块 */
__declspec(align(16)) struct STransation
{
	int keepWhite;      // 转场时是否保留白色区域：0=不保留，非0=保留
}; // 转场

/**
 * @brief 统一的 GPU 特效参数容器
 *
 * 通过 type 字段标识当前生效的特效类型，匿名联合体中同时持有所有特效的参数块，
 * 着色器根据 type 读取对应成员。仅一个特效参数块在任意时刻有效。
 */
__declspec(align(16)) struct CBXEffect
{
	int type = EEffectType::None;   // 当前生效的特效类型，对应 EEffectType 枚举值

	/** @brief 各特效 GPU 参数的匿名联合体，与 type 字段配合使用 */
	__declspec(align(16)) struct
	{
		SBlur blur;                 // 模糊参数
		SEmboss emboss;             // 浮雕参数
		SSharpen sharpen;           // 锐化参数
		SDenoise  denoise;          // 降噪参数
		SFrame  frame;              // 边框参数
		SFeather feather;           // 羽化参数
		SKeying keying;             // 抠像参数
		SBlink blink;               // 闪屏参数
		SRgba rgba;                 // RGBA 调整参数
		SFilter filter;             // 滤色参数
		SMosaic mosaic;             // 马赛克参数
		SShadow shadow;             // 阴影参数
		SHsv hsv;                   // HSV 调整参数
		SSpherize spherize;         // 球面化参数
		STwirl twirl;               // 漩涡参数
		SMirror mirror;             // 万花筒参数
		SNightvision nightvision;   // 夜视参数
		SMarquess marquees;         // 跑马灯参数
		SMask mask;                 // 蒙版参数
		STransation transation;     // 转场参数
	};
};

/*------------------------------------------------------------------------- 特效参数 -------------------------------------------------------------------------------*/

/** @brief 跑马灯特效支持的最大颜色数量 */
#define Max_Marquees_Color_Count 20

/**
 * @brief 特效参数基类
 *
 * 所有面向 UI/业务层的特效参数结构均继承此类，提供：
 * - 可见性控制（visiable）
 * - 特效类型标识（type）
 * - 向 GPU 参数 CBXEffect 的转换（ToCBXEffect）
 * - JSON 序列化/反序列化（SaveDataToJson / LoadDataFromJson）
 */
struct SFxFactor
{
	bool visiable = true;           // 特效是否可见/启用，false 时 ToCBXEffect 返回 None 类型
	int type = EEffectType::None;   // 特效类型，对应 EEffectType 枚举值

	/**
	 * @brief 将 UI 层参数转换为 GPU 常量缓冲区参数
	 * @return CBXEffect 结构，visiable 为 false 时 type 为 None
	 */
	virtual CBXEffect ToCBXEffect()
	{
		CBXEffect p;
		if (visiable)
		{
			p.type = type;
		}
		return p;
	};

	/**
	 * @brief 将参数序列化为 JSON 对象
	 * @return 包含 type 和 visiable 字段的 QJsonObject
	 */
	virtual QJsonObject SaveDataToJson()
	{
		QJsonObject Obj;
		Obj.insert("type", type);
		Obj.insert("visiable", visiable);
		return Obj;
	}

	/**
	 * @brief 从 JSON 对象反序列化参数
	 * @param obj 包含 type 和 visiable 字段的 JSON 对象
	 */
	virtual void LoadDataFromJson(const QJsonObject &obj)
	{
		type = obj.value("type").toInt();
		visiable = obj.value("visiable").toBool();
	}
};

/** @brief 蒙版形状类型枚举 */
enum EMaskType
{
	EMT_NoneType,       // 无蒙版/未指定
	EMT_CustomLayer,    // 自定义图层/图片蒙版
	EMT_3,              // 三角形蒙版
	EMT_flower,         // 花形蒙版
	EMT_4,              // 菱形蒙版
	EMT_heart,          // 心形蒙版
	EMT_6,              // 六边形蒙版
	EMT_ellipse,        // 椭圆形蒙版
	EMT_5,              // 五角星蒙版
	EMT_rrect,          // 圆角矩形蒙版
	EMT_circle,         // 圆形蒙版
	EMT_Num,            // 蒙版类型总数（哨兵值）
};

/** @brief 模糊特效 UI 层参数 */
struct SBlurFactor :public SFxFactor
{
	float degree = 0.5f;    // 模糊程度，范围 0.0~1.0，值越大越模糊
	float scale = 1.0f;   // 内部缩放比例，由 ToCBXEffect 根据 degree 自动计算
	int pass = 0;         // 模糊迭代趟数，由 ToCBXEffect 根据 scale 自动计算
	int sizeIndex = 0;    // 模糊核尺寸索引，由 ToCBXEffect 根据 scale 自动计算

	/** @brief 构造函数，初始化特效类型为 Blur，默认模糊程度 0.5 */
	SBlurFactor()
	{
		type = EEffectType::Blur;
		degree = 0.5f;
	}

	/**
	 * @brief 将 UI 模糊参数转换为 GPU 参数
	 * @return CBXEffect，scale>=1 或不可见时返回 None 类型
	 */
	virtual CBXEffect ToCBXEffect()
	{
		CBXEffect data;
		if (visiable)
		{
			double temp = pow(degree, 0.4f);

			scale = 1.0f - log10(temp * 9.0f + 1.0f);
			if (scale >= 1.0f)
			{
				data.type = EEffectType::None;
				return data;
			}

			pass = std::max(1, (int)round(4 * (1.0f - scale)));
			sizeIndex = (int)round(2 * (1.0f - scale));

			data.type = EEffectType::Blur;
			data.blur.sizeIndex = sizeIndex;
		}

		return data;
	};

	/** @brief 序列化模糊参数到 JSON */
	QJsonObject SaveDataToJson()
	{
		QJsonObject Obj = __super::SaveDataToJson();

		Obj.insert("degree", degree);

		return Obj;
	}

	/** @brief 从 JSON 反序列化模糊参数 */
	void LoadDataFromJson(const QJsonObject &obj)
	{
		__super::LoadDataFromJson(obj);

		degree = obj.value("degree").toDouble();
	}
};

/** @brief 浮雕特效 UI 层参数 */
struct SEmbossFactor :public SFxFactor
{
	int dir = 0;            // 光照方向，0~7：从左上角开始顺时针 8 个方向
	float weight = 0.2f;    // 浮雕加深程度，UI 范围 0~1，传入 GPU 时乘以 5

	/** @brief 构造函数，初始化特效类型为 Emboss，默认强度 0.2 */
	SEmbossFactor()
	{
		type = EEffectType::Emboss;
		weight = 0.2f;
	}

	/** @brief 将 UI 浮雕参数转换为 GPU 参数 */
	virtual CBXEffect ToCBXEffect()
	{
		CBXEffect data;
		if (visiable)
		{
			data.type = EEffectType::Emboss;
			data.emboss.dir = dir;
			data.emboss.weight = weight * 5;
		}

		return data;
	};

	/** @brief 序列化浮雕参数到 JSON */
	QJsonObject SaveDataToJson()
	{
		QJsonObject Obj = __super::SaveDataToJson();

		Obj.insert("dir", dir);
		Obj.insert("weight", weight);

		return Obj;
	}

	/** @brief 从 JSON 反序列化浮雕参数 */
	void LoadDataFromJson(const QJsonObject &obj)
	{
		__super::LoadDataFromJson(obj);

		dir = obj.value("dir").toInt();
		weight = obj.value("weight").toDouble();
	}
};

/** @brief 锐化特效 UI 层参数 */
struct SSharpenFactor :public SFxFactor
{
	float weight;   // 锐化强度，UI 范围 0.0~1.0，传入 GPU 时乘以 5

	/** @brief 构造函数，初始化特效类型为 Sharpen，默认强度 0.2 */
	SSharpenFactor()
	{
		type = EEffectType::Sharpen;
		weight = 0.2f;
	}

	/** @brief 将 UI 锐化参数转换为 GPU 参数 */
	virtual CBXEffect ToCBXEffect()
	{
		CBXEffect data;
		if (visiable)
		{
			data.type = EEffectType::Sharpen;
			data.sharpen.weight = weight * 5;
		}

		return data;
	};

	/** @brief 序列化锐化参数到 JSON */
	QJsonObject SaveDataToJson()
	{
		QJsonObject Obj = __super::SaveDataToJson();

		Obj.insert("weight", weight);

		return Obj;
	}

	/** @brief 从 JSON 反序列化锐化参数 */
	void LoadDataFromJson(const QJsonObject &obj)
	{
		__super::LoadDataFromJson(obj);

		weight = obj.value("weight").toDouble();
	}
};

/** @brief 降噪特效 UI 层参数 */
struct SDenoiseFactor :public SFxFactor
{
	int denoisetype;    // 降噪算法类型：0=椒盐噪声，1=高斯噪声
	int weight;         // 降噪强度，范围 0~4

	/** @brief 构造函数，初始化特效类型为 Denoise，默认椒盐降噪、强度 2 */
	SDenoiseFactor()
	{
		type = EEffectType::Denoise;
		denoisetype = 0;
		weight = 2;
	}

	/** @brief 将 UI 降噪参数转换为 GPU 参数 */
	virtual CBXEffect ToCBXEffect()
	{
		CBXEffect data;
		if (visiable)
		{
			data.type = EEffectType::Denoise;
			data.denoise.weight = weight;
			data.denoise.type = denoisetype;
		}

		return data;
	};

	/** @brief 序列化降噪参数到 JSON */
	QJsonObject SaveDataToJson()
	{
		QJsonObject Obj = __super::SaveDataToJson();

		Obj.insert("weight", weight);
		Obj.insert("denoisetype", denoisetype);

		return Obj;
	}

	/** @brief 从 JSON 反序列化降噪参数 */
	void LoadDataFromJson(const QJsonObject &obj)
	{
		__super::LoadDataFromJson(obj);

		weight = obj.value("weight").toInt();
		denoisetype = obj.value("denoisetype").toInt();
	}
}; // 降噪

/** @brief 闪屏特效 UI 层参数 */
struct SBlinkFactor :public SFxFactor
{
	float freq;     // 闪烁频率（次/帧或归一化频率）
	float last;     // 单次闪烁持续时间占比
	float shadow;   // 阴影/暗化程度
	float r, g, b;  // 闪烁覆盖层颜色 RGB 分量，范围 0~1

	/** @brief 构造函数，初始化特效类型为 Blink，默认白色、频率 0.05、持续 0.5 */
	SBlinkFactor()
	{
		type = EEffectType::Blink;
		r = 1.0f;
		g = 1.0f;
		b = 1.0f;
		freq = 0.05f;
		last = 0.5f;
		shadow = 0.0f;
	}

	/** @brief 将 UI 闪屏参数转换为 GPU 参数（仅传递 RGB，频率/持续由 CPU 端动画驱动） */
	virtual CBXEffect ToCBXEffect()
	{
		CBXEffect data;
		if (visiable)
		{
			data.type = EEffectType::Blink;
			data.blink.r = r;
			data.blink.g = g;
			data.blink.b = b;
		}

		return data;
	};

	/** @brief 序列化闪屏参数到 JSON */
	QJsonObject SaveDataToJson()
	{
		QJsonObject Obj = __super::SaveDataToJson();

		Obj.insert("r", r);
		Obj.insert("g", g);
		Obj.insert("b", b);
		Obj.insert("freq", freq);
		Obj.insert("last", last);
		Obj.insert("shadow", shadow);

		return Obj;
	}

	/** @brief 从 JSON 反序列化闪屏参数 */
	void LoadDataFromJson(const QJsonObject &obj)
	{
		__super::LoadDataFromJson(obj);

		r = obj.value("r").toDouble();
		g = obj.value("g").toDouble();
		b = obj.value("b").toDouble();
		freq = obj.value("freq").toDouble();
		last = obj.value("last").toDouble();
		shadow = obj.value("shadow").toDouble();
	}
};

/** @brief 羽化特效 UI 层参数 */
struct SFeatherFactor :public SFxFactor
{
	float size;     // 羽化范围/过渡带宽度，范围 0~1

	/** @brief 构造函数，初始化特效类型为 Feather，默认羽化 0.3 */
	SFeatherFactor()
	{
		type = EEffectType::Feather;
		size = 0.3f;
	}

	/** @brief 将 UI 羽化参数转换为 GPU 参数（size<=0 时为 None，避免 xe_feather 除零导致全黑） */
	virtual CBXEffect ToCBXEffect()
	{
		CBXEffect data;
		if (visiable)
		{
			// Radius<=0 不启用羽化
			if (size <= 0.0f)
				return data;

			data.type = EEffectType::Feather;
			data.feather.size = size;
		}

		return data;
	};

	/** @brief 序列化羽化参数到 JSON */
	QJsonObject SaveDataToJson()
	{
		QJsonObject Obj = __super::SaveDataToJson();

		Obj.insert("size", size);

		return Obj;
	}

	/** @brief 从 JSON 反序列化羽化参数 */
	void LoadDataFromJson(const QJsonObject &obj)
	{
		__super::LoadDataFromJson(obj);

		size = obj.value("size").toDouble();
	}
};

/** @brief RGBA 通道调整 UI 层参数（无 ToCBXEffect 实现，由外部直接读取字段） */
struct SRGBFactor :public SFxFactor
{
	float r, g, b, a;   // 各通道增益/偏移量，范围 0~1
};

/** @brief 抖动特效 UI 层参数 */
struct SShakeFactor :public SFxFactor
{
	float xOffset;      // 水平方向最大偏移量（归一化）
	float yOffset;      // 垂直方向最大偏移量（归一化）
	float frequency;    // 抖动频率，UI 范围 0~1，ToEffect 时平方后乘以 5

	/** @brief 构造函数，初始化特效类型为 Shake，默认水平偏移 0.02、频率 0.5 */
	SShakeFactor()
	{
		type = EEffectType::Shake;
		xOffset = 0.02f;
		yOffset = 0.0f;
		frequency = 0.5f;
	}

	/**
	 * @brief 将 UI 抖动参数转换为运行时特效参数（含频率非线性映射）
	 * @return 转换后的 SShakeFactor 副本
	 */
	SShakeFactor ToEffect()
	{
		SShakeFactor data;
		if (visiable)
		{
			data = *(this);
			data.frequency = pow(frequency, 2.0f)*5.0f;
			data.xOffset = xOffset;
			data.yOffset = yOffset;
		}

		return data;
	};

	/** @brief 序列化抖动参数到 JSON */
	QJsonObject SaveDataToJson()
	{
		QJsonObject Obj = __super::SaveDataToJson();

		Obj.insert("xOffset", xOffset);
		Obj.insert("yOffset", yOffset);
		Obj.insert("frequency", frequency);

		return Obj;
	}

	/** @brief 从 JSON 反序列化抖动参数 */
	void LoadDataFromJson(const QJsonObject &obj)
	{
		__super::LoadDataFromJson(obj);

		xOffset = obj.value("xOffset").toDouble();
		yOffset = obj.value("yOffset").toDouble();
		frequency = obj.value("frequency").toDouble();
	}
};

/** @brief 裁剪特效 UI 层参数 */
struct SCropFactor :public SFxFactor		//裁剪
{
	float l, t, r, b;   // 裁剪边距：左(l)、上(t)、右(r)、下(b)，可为像素或归一化比例
	bool stretch;       // 裁剪后是否拉伸填充目标区域
	bool bRadio;        // 当前 l/t/r/b 是否为归一化比例（true=比例，false=像素，需配合 w/h 转换）
	int w, h;           // 参考画面宽度/高度（像素），用于像素与比例互转

	/** @brief 构造函数，初始化特效类型为 Cut，默认无裁剪、保持宽高比、参考 1920x1080 */
	SCropFactor()
	{
		type = EEffectType::Cut;
		l = 0.f;
		r = 0.f;
		t = 0.f;
		b = 0.f;
		stretch = false;
		bRadio = true;
		w = 1920;
		h = 1080;
	}

	/** @brief 将像素单位的裁剪边距转换为归一化比例（当 bRadio 为 false 时生效） */
	void SetDataToRadio()
	{
		if (!bRadio)
		{
			l = l / w;
			r = r / w;
			t = t / h;
			b = b / h;
		}

	};

	/** @brief 将归一化比例的裁剪边距转换为像素单位（当 bRadio 为 false 时生效） */
	void SetDataToPx()
	{
		if (!bRadio)
		{
			l = l * w;
			r = r * w;
			t = t * h;
			b = b * h;
		}
	};

	/**
	 * @brief 将 UI 裁剪参数转换为运行时特效参数（自动处理像素/比例转换）
	 * @return 归一化后的 SCropFactor 副本
	 */
	SCropFactor ToEffect()
	{
		SCropFactor data;
		if (visiable)
		{
			data = *(this);
			if (!bRadio)
			{
				data.l = l / w;
				data.r = r / w;
				data.t = t / h;
				data.b = b / h;
			}
		}

		return data;
	};

	/** @brief 序列化裁剪参数到 JSON */
	QJsonObject SaveDataToJson()
	{
		QJsonObject Obj = __super::SaveDataToJson();

		Obj.insert("l", l);
		Obj.insert("r", r);
		Obj.insert("t", t);
		Obj.insert("b", b);
		Obj.insert("stretch", stretch);
		Obj.insert("bRadio", bRadio);
		Obj.insert("w", w);
		Obj.insert("h", h);

		return Obj;
	}

	/** @brief 从 JSON 反序列化裁剪参数 */
	void LoadDataFromJson(const QJsonObject &obj)
	{
		__super::LoadDataFromJson(obj);

		l = obj.value("l").toDouble();
		r = obj.value("r").toDouble();
		t = obj.value("t").toDouble();
		b = obj.value("b").toDouble();
		stretch = obj.value("stretch").toBool();
		bRadio = obj.value("bRadio").toBool();
		w = obj.value("w").toInt();
		h = obj.value("h").toInt();
	}
};

/** @brief 填充模式 UI 层参数（控制纹理在目标区域的缩放/对齐方式） */
struct SFillFactor :public SFxFactor	//填充模式
{
	bool		keepRatio;  // 是否保持源纹理宽高比
	bool		noScale;    // 是否禁止缩放（按原始尺寸显示）
	QSize		szSrc;      // 源纹理尺寸（像素）
	QSize		szDst;      // 目标显示区域尺寸（像素）
};

/** @brief 滤色特效 UI 层参数 */
struct SFilterFactor :public SFxFactor
{
	float r, g, b;  // 滤色目标颜色 RGB 分量，范围 0~1

	/** @brief 构造函数，初始化特效类型为 Filter，默认绿色 (0, 1, 0) */
	SFilterFactor()
	{
		type = EEffectType::Filter;
		r = 1.0;
		g = 1.0;
		b = 1.0;
	}

	/** @brief 将 UI 滤色参数转换为 GPU 参数 */
	virtual CBXEffect ToCBXEffect()
	{
		CBXEffect data;
		if (visiable)
		{
			data.type = EEffectType::Filter;
			data.filter.r = r;
			data.filter.g = g;
			data.filter.b = b;
		}

		return data;
	};

	/** @brief 序列化滤色参数到 JSON */
	QJsonObject SaveDataToJson()
	{
		QJsonObject Obj = __super::SaveDataToJson();

		Obj.insert("r", r);
		Obj.insert("g", g);
		Obj.insert("b", b);

		return Obj;
	}

	/** @brief 从 JSON 反序列化滤色参数 */
	void LoadDataFromJson(const QJsonObject &obj)
	{
		__super::LoadDataFromJson(obj);

		r = obj.value("r").toDouble();
		g = obj.value("g").toDouble();
		b = obj.value("b").toDouble();
	}
};

/** @brief 滚动特效 UI 层参数 */
struct SScrollFactor :public SFxFactor
{
	int dir;        // 滚动方向（0=上，1=下，2=左，3=右，具体值由着色器定义）
	float speed;    // 滚动速度（归一化，值越大越快）

	/** @brief 构造函数，初始化特效类型为 Scroll，默认向上、速度 0.2 */
	SScrollFactor()
	{
		type = EEffectType::Scroll;
		dir = 0;
		speed = 0.2f;
	}

	/**
	 * @brief 将 UI 滚动参数转换为运行时特效参数
	 * @return SScrollFactor 副本
	 */
	SScrollFactor ToEffect()
	{
		SScrollFactor data;
		if (visiable)
		{
			data = *(this);
		}

		return data;
	};

	/** @brief 序列化滚动参数到 JSON */
	QJsonObject SaveDataToJson()
	{
		QJsonObject Obj = __super::SaveDataToJson();

		Obj.insert("dir", dir);
		Obj.insert("speed", speed);

		return Obj;
	}

	/** @brief 从 JSON 反序列化滚动参数 */
	void LoadDataFromJson(const QJsonObject &obj)
	{
		__super::LoadDataFromJson(obj);

		dir = obj.value("dir").toInt();
		speed = obj.value("speed").toDouble();
	}
};

/** @brief 抠像特效 UI 层参数 */
struct SKeyingFactor :public SFxFactor
{
	float r, g, b;          // 抠像目标颜色 RGB 分量，范围 0~1
	float threshold;        // 色度匹配阈值，决定抠像灵敏度
	float weight;           // Y（亮度）分量权重，范围 0~1

	/** @brief 构造函数，初始化特效类型为 Keying，默认绿色、阈值 0.5、Y权重 0.1 */
	SKeyingFactor()
	{
		type = EEffectType::Keying;
		threshold = 0.5f;
		r = 0.0f;
		g = 1.0f;
		b = 0.0f;
		weight = 0.1f;
	}

	/** @brief 将 UI 抠像参数转换为 GPU 参数 */
	virtual CBXEffect ToCBXEffect()
	{
		CBXEffect data;
		if (visiable)
		{
			data.type = EEffectType::Keying;
			data.keying.r = r;
			data.keying.g = g;
			data.keying.b = b;
			data.keying.weight = weight;
			data.keying.threshold = threshold;
		}

		return data;
	};

	/** @brief 序列化抠像参数到 JSON */
	QJsonObject SaveDataToJson()
	{
		QJsonObject Obj = __super::SaveDataToJson();

		Obj.insert("threshold", threshold);
		Obj.insert("r", r);
		Obj.insert("g", g);
		Obj.insert("b", b);
		Obj.insert("weight", weight);

		return Obj;
	}

	/** @brief 从 JSON 反序列化抠像参数 */
	void LoadDataFromJson(const QJsonObject &obj)
	{
		__super::LoadDataFromJson(obj);

		threshold = obj.value("threshold").toDouble();
		r = obj.value("r").toDouble();
		g = obj.value("g").toDouble();
		b = obj.value("b").toDouble();
		weight = obj.value("weight").toDouble();
	}
};

/** @brief 夜视特效 UI 层参数 */
struct SNightViewFactor :public SFxFactor
{
	bool noise;     // 是否叠加噪点以增强夜视感

	/** @brief 构造函数，初始化特效类型为 Nightvision，默认开启噪点 */
	SNightViewFactor()
	{
		type = EEffectType::Nightvision;
		noise = true;
	}

	/** @brief 将 UI 夜视参数转换为 GPU 参数 */
	virtual CBXEffect ToCBXEffect()
	{
		CBXEffect data;
		if (visiable)
		{
			data.type = EEffectType::Nightvision;
			data.nightvision.noise = noise;
		}

		return data;
	};

	/** @brief 序列化夜视参数到 JSON */
	QJsonObject SaveDataToJson()
	{
		QJsonObject Obj = __super::SaveDataToJson();

		Obj.insert("noise", noise);

		return Obj;
	}

	/** @brief 从 JSON 反序列化夜视参数 */
	void LoadDataFromJson(const QJsonObject &obj)
	{
		__super::LoadDataFromJson(obj);

		noise = obj.value("noise").toBool();
	}
};

/** @brief 马赛克特效 UI 层参数 */
struct SMosaicFactor :public SFxFactor
{
	float x1, y1, x2, y2;   // 马赛克区域左上角与右下角，归一化坐标 0~1
	float size;             // 马赛克块大小，范围 0~1，传入 GPU 时除以 20

	float r, g, b;          // 马赛克区域边框颜色 RGB（UI 显示用，不传入 GPU）
	float width;            // 马赛克区域边框宽度（UI 显示用，不传入 GPU）

	/** @brief 构造函数，初始化特效类型为 Mosaic，默认全屏、块大小 0.4 */
	SMosaicFactor()
	{
		type = EEffectType::Mosaic;
		x1 = 0.0f;
		y1 = 0.0f;
		x2 = 1.0f;
		y2 = 1.0f;
		size = 0.4f;
		r = 0.5f;
		g = 0.5f;
		b = 0.5f;
		width = 0.0f;

	}

	/** @brief 将 UI 马赛克参数转换为 GPU 参数 */
	virtual CBXEffect ToCBXEffect()
	{
		CBXEffect data;
		if (visiable)
		{
			data.mosaic.size = size / 20;
			if (data.mosaic.size <= 0)
				return data;

			data.type = EEffectType::Mosaic;
			data.mosaic.x1 = x1;
			data.mosaic.y1 = y1;
			data.mosaic.x2 = x2;
			data.mosaic.y2 = y2;
		}

		return data;
	};

	/** @brief 序列化马赛克参数到 JSON */
	QJsonObject SaveDataToJson()
	{
		QJsonObject Obj = __super::SaveDataToJson();

		Obj.insert("x1", x1);
		Obj.insert("y1", y1);
		Obj.insert("x2", x2);
		Obj.insert("y2", y2);
		Obj.insert("size", size);
		Obj.insert("r", r);
		Obj.insert("g", g);
		Obj.insert("b", b);
		Obj.insert("width", width);

		return Obj;
	}

	/** @brief 从 JSON 反序列化马赛克参数 */
	void LoadDataFromJson(const QJsonObject &obj)
	{
		__super::LoadDataFromJson(obj);

		x1 = obj.value("x1").toDouble();
		y1 = obj.value("y1").toDouble();
		x2 = obj.value("x2").toDouble();
		y2 = obj.value("y2").toDouble();
		size = obj.value("size").toDouble();
		r = obj.value("r").toDouble();
		g = obj.value("g").toDouble();
		b = obj.value("b").toDouble();
		width = obj.value("width").toDouble();
	}
};

/** @brief 阴影特效 UI 层参数 */
struct SShadowFactor :public SFxFactor
{
	float size;         // 阴影缩放大小比例
	float distance;     // 阴影与主体的距离（归一化）
	float rotate;       // 阴影投射角度（度）
	float blur;         // 阴影虚化程度，范围 0~0.5，传入 GPU 时除以 2

	float r, g, b, a;   // 阴影颜色 RGBA 分量，范围 0~1

	/** @brief 构造函数，初始化特效类型为 Shadow，默认白色半透明、距离 0.1、角度 45° */
	SShadowFactor()
	{
		type = EEffectType::Shadow;
		r = 1.0f;
		g = 1.0f;
		b = 1.0f;
		a = 0.6f;
		distance = 0.1f;
		rotate = 45.f;
		blur = 0.2f;
		size = 1.15f;
	}

	/** @brief 将 UI 阴影参数转换为 GPU 参数 */
	virtual CBXEffect ToCBXEffect()
	{
		CBXEffect data;
		if (visiable)
		{
			data.type = EEffectType::Shadow;
			data.shadow.a = a;
			data.shadow.r = r;
			data.shadow.g = g;
			data.shadow.b = b;
			data.shadow.blur = blur / 2.f;
		}

		return data;
	};

	/** @brief 序列化阴影参数到 JSON */
	QJsonObject SaveDataToJson()
	{
		QJsonObject Obj = __super::SaveDataToJson();

		Obj.insert("a", a);
		Obj.insert("r", r);
		Obj.insert("g", g);
		Obj.insert("b", b);
		Obj.insert("size", size);
		Obj.insert("distance", distance);
		Obj.insert("rotate", rotate);
		Obj.insert("blur", blur);

		return Obj;
	}

	/** @brief 从 JSON 反序列化阴影参数 */
	void LoadDataFromJson(const QJsonObject &obj)
	{
		__super::LoadDataFromJson(obj);

		a = obj.value("a").toDouble();
		r = obj.value("r").toDouble();
		g = obj.value("g").toDouble();
		b = obj.value("b").toDouble();
		size = obj.value("size").toDouble();
		distance = obj.value("distance").toDouble();
		rotate = obj.value("rotate").toDouble();
		blur = obj.value("blur").toDouble();
	}
};

/** @brief HSV 色轮/色相循环特效 UI 层参数 */
struct SClrWheelFactor :public SFxFactor
{
	float cycle = 6.f;  // 色相循环周期（秒），值越小变化越快

	/** @brief 构造函数，初始化特效类型为 HSV，默认周期 6 秒 */
	SClrWheelFactor()
	{
		type = EEffectType::HSV;
		cycle = 6.f;
	}

	/** @brief 将 UI 色轮参数转换为 GPU 参数（仅设置色相偏移，S/V 固定为 0） */
	virtual CBXEffect ToCBXEffect()
	{
		CBXEffect data;
		if (visiable)
		{
			data.type = EEffectType::HSV;
			data.hsv.h = pow(1.3f, 11.0f - cycle);
			data.hsv.s = 0.0f;
			data.hsv.v = 0.0f;
		}

		return data;
	};

	/** @brief 序列化色轮参数到 JSON */
	QJsonObject SaveDataToJson()
	{
		QJsonObject Obj = __super::SaveDataToJson();

		Obj.insert("cycle", cycle);

		return Obj;
	}

	/** @brief 从 JSON 反序列化色轮参数 */
	void LoadDataFromJson(const QJsonObject &obj)
	{
		__super::LoadDataFromJson(obj);

		cycle = obj.value("cycle").toDouble();
	}
};

/** @brief 球面化（鱼眼）扭曲 UI 层参数 */
struct SSpherizeFactor :public SFxFactor
{
	float r, n;             // r=影响半径(0~1)，n=球面深度系数
	float x, y;             // 扭曲中心点，归一化坐标 0~1
	int clearOutside;       // 是否清除球面外部区域：0=保留，非0=清除

	/** @brief 构造函数，初始化特效类型为 Spherize，默认半径 0.7、深度 1、居中、清除外部 */
	SSpherizeFactor()
	{
		type = EEffectType::Spherize;
		r = 0.7f;
		n = 1.f;
		x = 0.5f;
		y = 0.5f;
		clearOutside = 1;
	}

	/** @brief 将 UI 球面化参数转换为 GPU 参数（计算椭球三轴半径） */
	virtual CBXEffect ToCBXEffect()
	{
		CBXEffect data;
		if (visiable)
		{
			if (r <= 0.0f || n <= 0.0f)
				return data;

			float N = n + 0.6f;
			float R = r * 0.5f * 1.41421356f;

			data.type = EEffectType::Spherize;
			data.spherize.x = x;
			data.spherize.y = y;
			data.spherize.a = R;
			data.spherize.b = R;
			data.spherize.c = R * N;
			data.spherize.clearOutside = clearOutside;
		}

		return data;
	};

	/** @brief 序列化球面化参数到 JSON */
	QJsonObject SaveDataToJson()
	{
		QJsonObject Obj = __super::SaveDataToJson();

		Obj.insert("r", r);
		Obj.insert("n", n);
		Obj.insert("x", x);
		Obj.insert("y", y);
		Obj.insert("clearOutside", clearOutside);

		return Obj;
	}

	/** @brief 从 JSON 反序列化球面化参数 */
	void LoadDataFromJson(const QJsonObject &obj)
	{
		__super::LoadDataFromJson(obj);

		r = obj.value("r").toDouble();
		n = obj.value("n").toDouble();
		x = obj.value("x").toDouble();
		y = obj.value("y").toDouble();
		clearOutside = obj.value("cyclearOutsidecle").toInt();
	}
};

/** @brief 漩涡扭曲 UI 层参数 */
struct STwirlFactor :public SFxFactor
{
	float x, y;         // 漩涡中心点，归一化坐标 0~1
	float r;            // 漩涡影响半径，范围 0~1
	float angle;        // 旋转角度系数（UI 值，ToCBXEffect 时非线性映射为弧度）
	int bClockwise;     // 旋转方向：0=逆时针，非0=顺时针
	int clearOutside;   // 是否清除半径外部区域：0=保留，非0=清除

	/** @brief 构造函数，初始化特效类型为 Twirl，默认半径 0.7、角度 0.5、顺时针、清除外部 */
	STwirlFactor()
	{
		type = EEffectType::Twirl;
		r = 0.7f;
		angle = 0.5f;
		x = 0.5f;
		y = 0.5f;
		bClockwise = 1;
		clearOutside = 1;
	}

	/** @brief 将 UI 漩涡参数转换为 GPU 参数（计算实际弧度） */
	virtual CBXEffect ToCBXEffect()
	{
		CBXEffect data;
		if (visiable)
		{
			if (r <= 0.0f)
				return data;

			float N = pow(float(angle), 2.0f) * 10 * 3.1415926f;
			float R = r * 0.5f * 1.41421356f;

			data.type = EEffectType::Twirl;
			data.twirl.x = x;
			data.twirl.y = y;
			data.twirl.r = R;
			data.twirl.radian = bClockwise ? N : -N;
			data.twirl.clearOutside = clearOutside;
		}

		return data;
	};

	/** @brief 序列化漩涡参数到 JSON */
	QJsonObject SaveDataToJson()
	{
		QJsonObject Obj = __super::SaveDataToJson();

		Obj.insert("r", r);
		Obj.insert("angle", angle);
		Obj.insert("x", x);
		Obj.insert("y", y);
		Obj.insert("bClockwise", bClockwise);
		Obj.insert("clearOutside", clearOutside);

		return Obj;
	}

	/** @brief 从 JSON 反序列化漩涡参数 */
	void LoadDataFromJson(const QJsonObject &obj)
	{
		__super::LoadDataFromJson(obj);

		r = obj.value("r").toDouble();
		angle = obj.value("angle").toDouble();
		x = obj.value("x").toDouble();
		y = obj.value("y").toDouble();
		bClockwise = obj.value("bClockwise").toInt();
		clearOutside = obj.value("cyclearOutsidecle").toInt();
	}
};

/** @brief 万花筒/镜像对称 UI 层参数 */
struct SKaleidoscopeFactor :public SFxFactor
{
	float x, y;         // 对称中心点，归一化坐标 0~1
	float section;      // 扇区数量（UI 以整数表示，ToCBXEffect 时转换为弧度 = PI/section）
	float offset;       // 起始扇区旋转偏移（UI 以度为单位，ToCBXEffect 时转换为弧度）

	/** @brief 构造函数，初始化特效类型为 Mirror，默认 10 扇区、偏移 120°、居中 */
	SKaleidoscopeFactor()
	{
		type = EEffectType::Mirror;
		section = 10.f;
		offset = 120.f;
		x = 0.5f;
		y = 0.5f;
	}

	/** @brief 将 UI 万花筒参数转换为 GPU 参数 */
	virtual CBXEffect ToCBXEffect()
	{
		CBXEffect data;
		if (visiable)
		{
			data.mirror.x = x;
			data.mirror.y = y;
			data.mirror.section = 3.1415926f / section;
			data.mirror.offset = offset / 180.f*3.1415926f;

			if (data.mirror.section <= 0.0f || data.mirror.section >= 2 * 3.1415926f)
				return data;

			data.type = EEffectType::Mirror;
		}

		return data;
	};

	/** @brief 序列化万花筒参数到 JSON */
	QJsonObject SaveDataToJson()
	{
		QJsonObject Obj = __super::SaveDataToJson();

		Obj.insert("offset", offset);
		Obj.insert("section", section);
		Obj.insert("x", x);
		Obj.insert("y", y);

		return Obj;
	}

	/** @brief 从 JSON 反序列化万花筒参数 */
	void LoadDataFromJson(const QJsonObject &obj)
	{
		__super::LoadDataFromJson(obj);

		offset = obj.value("offset").toDouble();
		section = obj.value("section").toDouble();
		x = obj.value("x").toDouble();
		y = obj.value("y").toDouble();
	}
};

/** @brief 跑马灯/霓虹灯带 UI 层参数 */
struct SMarqueesFactor :public SFxFactor
{
	float size;         // 灯带宽度（占画面短边比例）
	float length;       // 灯带长度（单位：圈）
	bool dir;           // 旋转方向：true=顺时针，false=逆时针
	float speed;        // 旋转速度（单位：圈/秒）

	int nColorCount;                            // 当前使用的颜色数量
	QColor  colors[Max_Marquees_Color_Count];   // 灯带颜色数组，最多 Max_Marquees_Color_Count 种

	/** @brief 构造函数，初始化特效类型为 Marquees，默认 3 色（红/绿/蓝） */
	SMarqueesFactor()
	{
		type = EEffectType::Marquees;
		size = 0.02f;
		length = 5.f;
		dir = true;
		speed = 0.2f;
		nColorCount = 3;
		colors[0] = QColor(255, 0, 0);
		colors[1] = QColor(0, 255, 0);
		colors[2] = QColor(0, 0, 255);
	}

	/** @brief 将 UI 跑马灯参数转换为 GPU 参数（颜色数为 0 时返回 None） */
	virtual CBXEffect ToCBXEffect()
	{
		CBXEffect data;
		if (visiable)
		{
			data.type = nColorCount == 0 ? EEffectType::None : EEffectType::Marquees;
		}

		return data;
	};

	/** @brief 序列化跑马灯参数到 JSON（含颜色数组） */
	QJsonObject SaveDataToJson()
	{
		QJsonObject Obj = __super::SaveDataToJson();

		Obj.insert("size", size);
		Obj.insert("length", length);
		Obj.insert("dir", dir);
		Obj.insert("speed", speed);
		Obj.insert("nColorCount", nColorCount);

		for (int i = 0; i < nColorCount; i++)
		{
			QJsonObject color;
			color.insert("r", colors[i].red());
			color.insert("g", colors[i].green());
			color.insert("b", colors[i].blue());

			Obj.insert(QString::number(i), color);
		}

		return Obj;
	}

	/** @brief 从 JSON 反序列化跑马灯参数（含颜色数组） */
	void LoadDataFromJson(const QJsonObject &obj)
	{
		__super::LoadDataFromJson(obj);

		size = obj.value("size").toDouble();
		length = obj.value("length").toDouble();
		dir = obj.value("dir").toBool();
		speed = obj.value("speed").toDouble();
		nColorCount = obj.value("nColorCount").toDouble();

		for (int i = 0; i < nColorCount; i++)
		{
			QJsonObject color = obj.value(QString::number(i)).toObject();

			colors[i].setRed(color.value("r").toInt());
			colors[i].setGreen(color.value("g").toInt());
			colors[i].setBlue(color.value("b").toInt());
		}
	}

}; // 跑马灯

/** @brief 蒙版特效 UI 层参数 */
struct SMaskFactor :public SFxFactor
{
	int MaskType = EMT_NoneType;    // 蒙版形状类型，对应 EMaskType 枚举
	QString filename;               // 自定义蒙版图片文件路径（MaskType 为 EMT_CustomLayer 时使用）
	bool reverse;                   // 是否反转蒙版（显示蒙版外部区域）
	float x, y, w, h;               // 蒙版位置与尺寸：中心 (x,y) 及宽高 (w,h)，归一化坐标 0~1
	bool bDirty;                    // 蒙版纹理是否需要重新生成/上传

	/** @brief 构造函数，初始化特效类型为 Mask，默认五角星、居中、不反转 */
	SMaskFactor()
	{
		type = EEffectType::Mask;
		MaskType = EMT_5;
		reverse = false;
		x = 0.5f;
		y = 0.5f;
		w = 1.0f;
		h = 1.0f;
		filename = "-";
		bDirty = true;
	}

	/** @brief 将 UI 蒙版参数转换为 GPU 参数 */
	virtual CBXEffect ToCBXEffect()
	{
		CBXEffect data;
		if (visiable)
		{
			data.type = EEffectType::Mask;
			data.mask.reverse = reverse;
			data.mask.bPicture = MaskType == EMT_CustomLayer;
		}

		return data;
	};

	/** @brief 序列化蒙版参数到 JSON */
	QJsonObject SaveDataToJson()
	{
		QJsonObject Obj = __super::SaveDataToJson();

		Obj.insert("MaskType", MaskType);
		Obj.insert("reverse", reverse);
		Obj.insert("x", x);
		Obj.insert("y", y);
		Obj.insert("w", w);
		Obj.insert("h", h);
		Obj.insert("filename", filename);

		return Obj;
	}

	/** @brief 从 JSON 反序列化蒙版参数 */
	void LoadDataFromJson(const QJsonObject &obj)
	{
		__super::LoadDataFromJson(obj);

		MaskType = obj.value("MaskType").toInt();
		reverse = obj.value("reverse").toBool();
		x = obj.value("x").toDouble();
		y = obj.value("y").toDouble();
		w = obj.value("w").toDouble();
		h = obj.value("h").toDouble();
		filename = obj.value("filename").toString();
	}
};

/** @brief 边框特效 UI 层参数 */
struct SFrameFactor :public SFxFactor
{
	float size;         // 边框宽度（占画面短边比例），传入 GPU 时除以 2
	float r, g, b, a;   // 边框颜色 RGBA 分量，范围 0~1

	/** @brief 构造函数，初始化特效类型为 Frame，默认青色 (29,209,229) 半透明 */
	SFrameFactor()
	{
		type = EEffectType::Frame;	//29,209,229
		a = 0.8f;
		r = 0.114f;
		g = 0.82f;
		b = 0.9f;
		size = 0.2f;
	}

	/** @brief 将 UI 边框参数转换为 GPU 参数 */
	virtual CBXEffect ToCBXEffect()
	{
		CBXEffect data;
		if (visiable)
		{
			data.type = EEffectType::Frame;
			data.frame.r = r;
			data.frame.g = g;
			data.frame.b = b;
			data.frame.size = size / 2;
		}

		return data;
	};

	/** @brief 序列化边框参数到 JSON */
	QJsonObject SaveDataToJson()
	{
		QJsonObject Obj = __super::SaveDataToJson();

		Obj.insert("a", a);
		Obj.insert("r", r);
		Obj.insert("g", g);
		Obj.insert("b", b);
		Obj.insert("size", size);

		return Obj;
	}

	/** @brief 从 JSON 反序列化边框参数 */
	void LoadDataFromJson(const QJsonObject &obj)
	{
		__super::LoadDataFromJson(obj);

		a = obj.value("a").toDouble();
		r = obj.value("r").toDouble();
		g = obj.value("g").toDouble();
		b = obj.value("b").toDouble();
		size = obj.value("size").toDouble();
	}
}; // 边框

/** @brief 特效节点注册表条目总数 */
static const int g_effectNodeInfoNum = 22;

/** @brief 特效节点注册信息结构，用于特效树面板与节点工厂 */
static const struct SEffectNodeInfo
{
	int _infoIndex;			// 绘制/排序索引
	EEffectType _infoType;	// 特效类型枚举值
	QString _infoId;		// 特效唯一 GUID 标识符
	QString _infoStr;		// 特效显示名称（英文）
}g_effectNodeInfo[] = {
	{0	, EEffectType::Denoise		, QString("745E627C-C2F3-4811-887C-6EB0444E1436"),  QString("Denoise")},
	{1	, EEffectType::Sharpen		, QString("5766D525-FFF7-45C4-9212-5C746AA0E7B3"),  QString("Sharpen")},
	{2	, EEffectType::Blur			, QString("B7BFF781-8F80-4B6A-BDB9-36B8A79B02FF"),  QString("Blur")},
	{3	, EEffectType::Mosaic		, QString("12D434FD-F3D1-4B8E-BAE4-4D4E13041C1E"),  QString("Mosaic")},
	{4	, EEffectType::Complement	, QString("47C79D18-08CD-4EAD-AD11-3929991CEFC6"),  QString("Complement")},
	{5	, EEffectType::HSV			, QString("05E8CA02-33BB-4DB5-8F2E-1F486B87D8EC"),  QString("HSV")},
	{6	, EEffectType::Filter		, QString("8C7C1F5E-5028-4004-A3B7-E90BDEA9AA73"),  QString("Filter")},
	{7	, EEffectType::Nightvision	, QString("7E5060C8-483F-4FB1-967A-B83CFDBD6408"),  QString("NightView")},
	{8	, EEffectType::Emboss		, QString("4FBAF5BC-673E-46F1-9810-E2659D8027C5"),  QString("Emboss")},
	{9	, EEffectType::Spherize		, QString("5DCAED3A-00AD-429E-899B-1D8BC89AA54B"),  QString("Spherize")},
	{10	, EEffectType::Twirl		, QString("A492E7F1-FA29-4F96-AC57-294A80450FF3"),  QString("Twirl")},
	{11	, EEffectType::Mirror		, QString("99BB05D5-9928-4D7D-B55B-15CA412264B0"),  QString("Kaleidoscope")},
	{12	, EEffectType::Shadow		, QString("40AF697C-631F-4FD5-86A8-A52481D010FB"),  QString("Shadow")},
	{13	, EEffectType::Feather		, QString("3B939CC7-B0D0-42D2-A235-9C031FA20F1C"),  QString("Feather")},
	{14	, EEffectType::Cut			, QString("891C7B6A-CB05-4611-BA75-BFAB243243F2"),  QString("Crop")},
	{15	, EEffectType::Mask			, QString("C713D527-122F-4274-A683-BC22F5AC8930"),  QString("Mask")},
	{16	, EEffectType::Keying		, QString("846F0B58-A405-4F2A-89E5-73DE344F00FF"),  QString("Keying")},
	{17	, EEffectType::Frame		, QString("8D491B8C-905A-422F-9334-2896A87F05BF"),  QString("Frame")},
	{18	, EEffectType::Marquees		, QString("BFD929E2-C1AC-45B9-A22F-BA66CCDAA32D"),  QString("Marquees")},
	{19	, EEffectType::Scroll		, QString("FC010674-CD36-4626-93E2-5BD731DAF5BA"),  QString("Scroll")},
	{20	, EEffectType::Shake		, QString("633707D6-D50E-41FC-972F-D22964ABBBC0"),  QString("Shake")},
	{21	, EEffectType::Blink		, QString("3727C1EE-56D2-4729-9AF9-D51AEE0001A2"),  QString("Blink")}
};