#pragma once

#include "Player.h"
#include "PlayerGlobal.h"

#include <QString>
#include <QRunnable>

class CLoadTexTask : public QObject, public QRunnable
{
	Q_OBJECT
public:
	CLoadTexTask(CD3DResourceChild* pD3DRes, const QString& path, const int nExifOri);
	virtual ~CLoadTexTask();

	void run() override;

signals:
	void signaLoadFinish(const QString& path, void* handle);

protected:
	QString m_path;

	CD3DResourceChild* m_pD3DRes = nullptr;
	ComPtr<ID3D11Texture2D> m_pTex = nullptr;

	int m_nExifOrientation = 0;

};

class PLAYER_EXPORT CPicPlayer : public QObject, public CBasePlayer, public IPicPlayer
{
	Q_OBJECT
public:
	CPicPlayer(std::shared_ptr<IResourceBase> pResBase, const BasePlayerContext& ctx, const QString& strPath, int nExifOri);
	virtual ~CPicPlayer();

public:
	virtual bool Prepare();
	virtual void PictureTrans(const QString& path);

	virtual bool	Start(const microseconds& curTime) override;
	virtual bool	Stop() override;
	virtual bool	Restart(const microseconds& curTime) override;

public slots:
	void slotLoadTexFinish(const QString& path, void* handle);

protected:
	QString m_filePath;

	CLoadTexTask* m_pTrdLoadImage = nullptr;

	int m_nExifOrientation = 0;

	std::atomic<bool> m_bIsRuning = false;

protected:
	void Start(const QString& strpath);
};