#include "vk_pathes.h"

std::filesystem::path Utils::GetProjectRoot() {
    static const std::filesystem::path rootPath = []() {
        // 실행 파일의 경로를 가져옵니다
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);

        // 실행 파일 경로에서 디렉토리 부분만 추출
        std::filesystem::path executablePath(exePath);
        std::filesystem::path executableDir = executablePath.parent_path();

        // x64/Debug에서 두 단계 위로 올라간 후 VulkanTest 폴더로 들어갑니다
        std::filesystem::path projectDir = executableDir.parent_path().parent_path() / ProjectName;

        std::cerr << "finding rot project directory... " << projectDir << std::endl;

        // 유효성 검사 - 디렉토리가 존재하는지 확인
        if (std::filesystem::exists(projectDir)) {
            return projectDir;
        }

        // 찾지 못했다면 경고 메시지 출력 후 기본값 반환
        std::cerr << "Warning: Could not locate project root directory. Using fallback directory." << std::endl;
        return std::filesystem::current_path();
        }();

        return rootPath;
}