#pragma once
#include "Common.h"
#include <vector>
#include <functional>
#include <atomic>
#include <mutex>

namespace GK {

// ─────────────────────────────────────────────────────────────
//  TemplateManager
//
//  - versions.json["templates"] 를 파싱해서 TemplateInfo 목록 유지
//  - 썸네일: 앱 시작 직후 비동기로 다운로드 → Library/thumbnails/<id>.png
//  - 비내장(builtin=false) 템플릿 zip: 사용자 요청 시 다운로드 →
//    Library/templates/<id>/ 에 압축 해제 (영구 캐시)
//  - 프로젝트 생성 시 cacheDir 을 ProjectManager 에 전달하면
//    ProjectManager 가 복사 처리
// ─────────────────────────────────────────────────────────────
class TemplateManager {
public:
    static TemplateManager& instance();

    // versions.json 에서 파싱 (VersionManager::checkForUpdates 후 호출)
    void parseFromJson(const std::string& versionsJson);

    // 앱 시작 시 호출: 모든 썸네일 비동기 다운로드
    void prefetchThumbnails();

    // 비내장 템플릿 zip 다운로드 + 압축 해제 (비동기)
    void downloadTemplate(
        const std::string& templateId,
        ProgressCallback onProgress,
        std::function<void(bool ok, const std::string& err)> onDone);

    // 캐시 경로 스캔 (재시작 후 기존 캐시 인식)
    void scanLocalCache();

    const std::vector<TemplateInfo>& templates() const { return m_templates; }

    // 썸네일 다운로드 완료 콜백 (UI 갱신용)
    using ThumbCallback = std::function<void(const std::string& id)>;
    int  onThumbnailReady(ThumbCallback cb);
    void removeCallback(int id);

private:
    TemplateManager() = default;

    void downloadThumbnailAsync(TemplateInfo& tmpl);
    bool downloadFile(const std::string& url,
                      const std::filesystem::path& dest,
                      ProgressCallback onProgress,
                      std::string& outErr);

    std::vector<TemplateInfo> m_templates;
    std::mutex m_mutex;

    std::unordered_map<int, ThumbCallback> m_thumbCbs;
    int m_nextCbId = 0;
};

} // namespace GK
