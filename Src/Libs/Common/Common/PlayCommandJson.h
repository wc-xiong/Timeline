#pragma once

#include "PlayCommand.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QVariant>

inline QJsonObject ToJson(const SCellPlayState& state)
{
	QJsonObject j;
	j["idCell"] = state.idCell.toString();
	j["mode"] = state.mode;
	j["ready"] = state.ready;
	j["isNewest"] = state.isNewest;
	j["state"] = state.state;
	j["startTime"] = static_cast<qint64>(state.startTime.count());
	j["seek"] = static_cast<qint64>(state.seek);
	j["tcTime"] = static_cast<qint64>(state.tcTime);
	j["tcSpeed"] = state.tcSpeed;
	j["crossScreen"] = state.crossScreen;
	return j;
}

inline void FromJson(const QJsonObject& j, SCellPlayState& state)
{
	state.idCell = QUuid(j.value("idCell").toString());
	state.mode = j.value("mode").toInt(state.mode);
	state.ready = j.value("ready").toBool(state.ready);
	state.isNewest = j.value("isNewest").toBool(state.isNewest);
	state.state = j.value("state").toInt(state.state);
	state.startTime = std::chrono::microseconds(j.value("startTime").toVariant().toLongLong());
	state.seek = j.value("seek").toVariant().toLongLong();
	state.tcTime = j.value("tcTime").toVariant().toLongLong();
	state.tcSpeed = j.value("tcSpeed").toDouble(state.tcSpeed);
	state.crossScreen = j.value("crossScreen").toInt(state.crossScreen);
}

inline QJsonObject ToJson(const SIpPlayState& state)
{
	QJsonObject j;
	j["seq"] = static_cast<qint64>(state.seq);
	j["type"] = static_cast<int>(state.type);
	j["needCmdFinish"] = state.needCmdFinish;
	j["isIpReady"] = state.isIpReady;
	j["mode"] = state.mode;
	j["waitCell"] = ToJson(state.waitCell);
	return j;
}

inline void FromJson(const QJsonObject& j, SIpPlayState& state)
{
	state.seq = j.value("seq").toVariant().toLongLong();
	state.type = j.value("type").toInt();
	state.needCmdFinish = j.value("needCmdFinish").toBool(true);
	state.isIpReady = j.value("isIpReady").toBool(true);
	state.mode = j.value("mode").toInt(state.mode);
	if (j.contains("waitCell") && j["waitCell"].isObject())
		FromJson(j["waitCell"].toObject(), state.waitCell);
}

inline QJsonObject ToJson(const SGSPlayCommand& cmd)
{
	QJsonObject j;
	j["seq"] = static_cast<qint64>(cmd.seq);
	j["type"] = static_cast<int>(cmd.type);
	j["timestamp"] = static_cast<qint64>(cmd.timestamp.count());
	j["data"] = cmd.data;
	return j;
}

inline void FromJson(const QJsonObject& j, SGSPlayCommand& cmd)
{
	cmd.seq = j.value("seq").toVariant().toLongLong();
	cmd.type = static_cast<uint32_t>(j.value("type").toInt());
	cmd.timestamp = std::chrono::microseconds(j.value("timestamp").toVariant().toLongLong());
	cmd.data = j.value("data").toObject();
}