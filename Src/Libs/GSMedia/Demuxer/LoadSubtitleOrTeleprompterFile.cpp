#include "LoadSubtitleOrTeleprompterFile.h"
#include "Common/Common/CommonFunctionFileOp.h"
#include "ResDemuxer.h"
#include <QFile>

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTime>

#define DEFALUT_SUBTITLE_INTERVAL 2000
#define DEFAULT_ENDTEXT_DURATION 3000

CSubtitleDemuxer::CSubtitleDemuxer()
{}

CSubtitleDemuxer::~CSubtitleDemuxer()
{}

bool CSubtitleDemuxer::Demuxer(std::shared_ptr<SResFileDemuxInfo>&info)
{
	if (info->type == EResourceType::ERT_Subtitle)
	{
		LoadSubtitleFile(info);
	}
	else
	{
		LoadTeleprompterFile(info);
	}
	return true;
}

void CSubtitleDemuxer::LoadSubtitleFile(std::shared_ptr<SResFileDemuxInfo>& info)
{
	QString strPath = QString::fromStdString(info->path);
	QString strCodeType = GetFileCodeType(strPath);

	// 读取文件
	QFile file(strPath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;
	QTextStream stream(&file);
	stream.setCodec(strCodeType.toStdString().c_str());  //设置QTextStream对象的编码格式
	QString readStr = stream.readAll();
	file.close();

	// 通过后缀判断文件类型
	int nIndex = strPath.lastIndexOf('.');
	QString szSuffix = strPath.right(strPath.length() - nIndex - 1);
	szSuffix = szSuffix.toLower();

	__int64 nMediaDuration = -1;
	std::vector<SSubtitleDataNode> vecDatas;

	auto ReadLRCFormat = [&]() {
		QRegularExpression rex("\\[(\\d+?:\\d+\\.\\d+)?\\]([^\r\n]*)"); // 正则表达式
		QRegularExpressionMatch match = rex.match(readStr);
		if (!match.hasMatch())	// 不支持本格式
			return;

		QRegularExpressionMatchIterator globalMatch = rex.globalMatch(readStr);
		while (globalMatch.hasNext())
		{
			QRegularExpressionMatch item = globalMatch.next();
			SSubtitleDataNode node;
			for (int i = 0; i <= item.lastCapturedIndex(); i++)
			{
				auto strContent = item.captured(i);
				if (strContent.isEmpty())
					continue;

				switch (i)
				{
				case 0:	// 所有内容
					break;
				case 1: // 开始时间
				{
					QTime tBegin = QTime::fromString(strContent, "mm:ss.z");
					if (tBegin.msec() == -1)
						break;
					node.nBeginTime = (tBegin.minute() * 60 + tBegin.second()) * 1000 + tBegin.msec();
				}
				break;
				default: // 内容
				{
					if (!node.strText.isEmpty())
						node.strText += "\n";
					node.strText += strContent;
				}
				break;
				}
			}

			if (node.strText.isEmpty() && !vecDatas.empty())	// 有些格式为第二行只有时间，为上一行的显示结束时间
				vecDatas[vecDatas.size() - 1].nEndTime = node.nBeginTime;
			else if (node.nBeginTime != -1)
				vecDatas.push_back(node);
		}

		// 处理字幕结束时间，若是文件中未单独给出，则上一条字幕的结束时间为下下一条字幕的开始时间
		for (int i = (int)vecDatas.size() - 1; i >= 0; i--)
		{
			if (vecDatas[i].nEndTime == -1)
			{
				if (i == (int)vecDatas.size() - 1)
					vecDatas[i].nEndTime = vecDatas[i].nBeginTime + DEFAULT_ENDTEXT_DURATION;
				else
					vecDatas[i].nEndTime = vecDatas[i + 1].nBeginTime;
			}

			nMediaDuration = std::max(nMediaDuration, vecDatas[i].nEndTime);
		}
	};

	auto ReadSRTFormat = [&]() {
		QRegularExpression rex("(\\d+)?\n(\\d+:\\d+:\\d+,\\d+) --> (\\d+:\\d+:\\d+,\\d+)(\\d*(?:\r?(?!\r?).*)*)\n(.*(?:\r?\n(?!\r?\n).*)*)"); // 正则表达式
		QRegularExpressionMatch match = rex.match(readStr);
		if (!match.hasMatch())	// 不支持本格式
			return;

		QRegularExpressionMatchIterator globalMatch = rex.globalMatch(readStr);
		while (globalMatch.hasNext())
		{
			QRegularExpressionMatch item = globalMatch.next();
			SSubtitleDataNode node;
			for (int i = 0; i <= item.lastCapturedIndex(); i++)
			{
				auto strContent = item.captured(i);
				if (strContent.isEmpty())
					continue;

				switch (i)
				{
				case 0:	// 所有内容
					break;
				case 1:	// 序号
					break;
				case 2: // 开始时间
				{
					QTime tBegin = QTime::fromString(strContent, "hh:mm:ss,zzz");
					if (tBegin.msec() == -1)
						break;
					node.nBeginTime = (tBegin.hour() * 3600 + tBegin.minute() * 60 + tBegin.second()) * 1000 + tBegin.msec();
				}
				break;
				case 3: // 结束时间
				{
					QTime tEnd = QTime::fromString(strContent, "hh:mm:ss,zzz");
					if (tEnd.msec() == -1)
						break;
					node.nEndTime = (tEnd.hour() * 3600 + tEnd.minute() * 60 + tEnd.second()) * 1000 + tEnd.msec();
					nMediaDuration = std::max(nMediaDuration, node.nEndTime);
				}
				break;
				default: // 内容
				{
					if (!node.strText.isEmpty())
						node.strText += "\n";
					node.strText += strContent;
				}
				break;
				}
			}

			if (node.nBeginTime != -1 && node.nEndTime != -1)
				vecDatas.push_back(node);
		}
	};

	if (szSuffix == "lrc")
	{
		ReadLRCFormat();
	}
	else if (szSuffix == "srt")
	{
		ReadSRTFormat();
	}
	else if (szSuffix == "txt")
	{
		// 首先按照LRC文件格式读取
		ReadLRCFormat();

		// 首先按照SRT文件格式读取
		if (vecDatas.empty())
			ReadSRTFormat();

		if (vecDatas.empty())	// 直接读取每行，间隔两秒
		{
			int nBiginTime = 0;
			while (!readStr.isEmpty())
			{
				int nEnterIndex = readStr.indexOf('\n');
				if (nEnterIndex == -1)	// 最后一行
				{
					vecDatas.push_back(SSubtitleDataNode(nBiginTime, nBiginTime + DEFALUT_SUBTITLE_INTERVAL, readStr));
					break;
				}

				QString str = readStr.left(nEnterIndex);
				vecDatas.push_back(SSubtitleDataNode(nBiginTime, nBiginTime + DEFALUT_SUBTITLE_INTERVAL, str));

				readStr = readStr.mid(nEnterIndex + 1, readStr.length() - nEnterIndex); // 截取 已解析的字符串

				nBiginTime += DEFALUT_SUBTITLE_INTERVAL;
			}

			if (!vecDatas.empty())
			{
				nMediaDuration = (*vecDatas.rbegin()).nEndTime;
			}
		}
	}


	std::shared_ptr<SSubtitleInfo> demuxInfo = std::make_shared<SSubtitleInfo>();
	demuxInfo->id = info->id;
	demuxInfo->type = ERT_Subtitle;
	demuxInfo->nMediaDuration = nMediaDuration;
	demuxInfo->vecSubtitleDatas = vecDatas;
	emit g_pResDemuxer->signalDemuxerFinish(demuxInfo);
}

void CSubtitleDemuxer::LoadTeleprompterFile(std::shared_ptr<SResFileDemuxInfo>& info)
{
	QString strPath = QString::fromStdString(info->path);
	QString strCodeType = GetFileCodeType(strPath);

	// 读取文件
	QFile file(strPath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;
	QTextStream stream(&file);
	stream.setCodec(strCodeType.toStdString().c_str());  //设置QTextStream对象的编码格式
	QString readStr = stream.readAll();
	file.close();

	// 通过后缀判断文件类型
	int nIndex = strPath.lastIndexOf('.');
	QString szSuffix = strPath.right(strPath.length() - nIndex - 1);
	szSuffix = szSuffix.toLower();

	std::vector<STeleprompterItem> vecDatas;

	auto ReadLRCFormat = [&]() {

		QRegularExpression rex("\\[(\\d+?:\\d+\\.\\d+)?\\]([^\r\n]*)"); // 正则表达式
		QRegularExpressionMatch match = rex.match(readStr);
		if (!match.hasMatch())	// 不支持本格式
			return;

		QRegularExpressionMatchIterator globalMatch = rex.globalMatch(readStr);

		std::vector<__int64> vecTimeStramp;
		std::vector<QString> vecString;
		while (globalMatch.hasNext())
		{
			QRegularExpressionMatch item = globalMatch.next();
			for (int i = 0; i <= item.lastCapturedIndex(); i++)
			{
				auto strContent = item.captured(i);

				switch (i)
				{
				case 0:	// 所有内容
					break;
				case 1: // 开始时间
				{
					QTime tBegin = QTime::fromString(strContent, "mm:ss.z");
					if (tBegin.msec() == -1)
						break;

					__int64 timeStramp = (tBegin.minute() * 60 + tBegin.second()) * 1000 + tBegin.msec();
					vecTimeStramp.push_back(timeStramp);
				}
				break;
				default: // 内容
				{
					vecString.push_back(strContent);
				}
				break;
				}
			}
		}

		for (int i = 0; i < vecString.size(); i++)
		{
			STeleprompterItem node;
			node.strText = vecString[i];
			if (vecTimeStramp.empty())
				return;

			if (i <= vecString.size() - 2)
			{
				node.nDuration = vecTimeStramp[i + 1] - vecTimeStramp[i];
			}

			vecDatas.push_back(node);
		}
	};

	auto ReadSRTFormat = [&]() {
		QRegularExpression rex("(\\d+)?\n(\\d+:\\d+:\\d+,\\d+) --> (\\d+:\\d+:\\d+,\\d+)(\\d*(?:\r?(?!\r?).*)*)\n(.*(?:\r?\n(?!\r?\n).*)*)"); // 正则表达式
		QRegularExpressionMatch match = rex.match(readStr);
		if (!match.hasMatch())	// 不支持本格式
			return;

		QRegularExpressionMatchIterator globalMatch = rex.globalMatch(readStr);
		while (globalMatch.hasNext())
		{
			QRegularExpressionMatch item = globalMatch.next();
			STeleprompterItem node;
			__int64 beginTime;
			__int64 endTime;

			for (int i = 0; i <= item.lastCapturedIndex(); i++)
			{
				auto strContent = item.captured(i);
				if (strContent.isEmpty())
					continue;

				switch (i)
				{
				case 0:	// 所有内容
					break;
				case 1:	// 序号
					break;
				case 2: // 开始时间
				{
					QTime tBegin = QTime::fromString(strContent, "hh:mm:ss,zzz");
					if (tBegin.msec() == -1)
						break;

					beginTime = (tBegin.hour() * 3600 + tBegin.minute() * 60 + tBegin.second()) * 1000 + tBegin.msec();
				}
				break;
				case 3: // 结束时间
				{
					QTime tEnd = QTime::fromString(strContent, "hh:mm:ss,zzz");
					if (tEnd.msec() == -1)
						break;

					endTime = (tEnd.hour() * 3600 + tEnd.minute() * 60 + tEnd.second()) * 1000 + tEnd.msec();

					int dur = endTime - beginTime;
					if (dur > 0)
						node.nDuration = dur;
				}
				break;
				default: // 内容
				{
					if (!node.strText.isEmpty())
						node.strText += "\n";
					node.strText += strContent;
				}
				break;
				}
			}

			vecDatas.push_back(node);
		}
	};

	if (szSuffix == "lrc")
	{
		ReadLRCFormat();
	}
	else if (szSuffix == "srt")
	{
		ReadSRTFormat();
	}
	else if (szSuffix == "txt")
	{
		// 首先按照LRC文件格式读取
		ReadLRCFormat();

		// 首先按照SRT文件格式读取
		if (vecDatas.empty())
			ReadSRTFormat();

		if (vecDatas.empty())	// 直接读取每行，间隔两秒
		{
			while (!readStr.isEmpty())
			{
				int nEnterIndex = readStr.indexOf('\n');
				if (nEnterIndex == -1)	// 最后一行
				{
					vecDatas.push_back(STeleprompterItem(readStr));
					break;
				}

				QString str = readStr.left(nEnterIndex);
				vecDatas.push_back(STeleprompterItem(str));

				readStr = readStr.mid(nEnterIndex + 1, readStr.length() - nEnterIndex); // 截取 已解析的字符串
			}
		}
	}

	std::shared_ptr<STeleprompterInfo> demuxInfo = std::make_shared<STeleprompterInfo>();
	demuxInfo->id = info->id;
	demuxInfo->type = ERT_Teleprompter;
	demuxInfo->vecTeleprompterDatas = vecDatas;
	emit g_pResDemuxer->signalDemuxerFinish(demuxInfo);
}