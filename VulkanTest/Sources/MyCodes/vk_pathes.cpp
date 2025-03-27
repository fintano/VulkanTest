#include "vk_pathes.h"

std::filesystem::path Utils::GetProjectRoot() {
    static const std::filesystem::path rootPath = []() {
        // ���� ������ ��θ� �����ɴϴ�
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);

        // ���� ���� ��ο��� ���丮 �κи� ����
        std::filesystem::path executablePath(exePath);
        std::filesystem::path executableDir = executablePath.parent_path();

        // x64/Debug���� �� �ܰ� ���� �ö� �� VulkanTest ������ ���ϴ�
        std::filesystem::path projectDir = executableDir.parent_path().parent_path() / ProjectName;

        std::cerr << "finding rot project directory... " << projectDir << std::endl;

        // ��ȿ�� �˻� - ���丮�� �����ϴ��� Ȯ��
        if (std::filesystem::exists(projectDir)) {
            return projectDir;
        }

        // ã�� ���ߴٸ� ��� �޽��� ��� �� �⺻�� ��ȯ
        std::cerr << "Warning: Could not locate project root directory. Using fallback directory." << std::endl;
        return std::filesystem::current_path();
        }();

        return rootPath;
}