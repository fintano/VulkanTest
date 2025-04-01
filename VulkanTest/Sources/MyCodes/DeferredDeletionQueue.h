#pragma once
#include <vector>
#include <memory>
#include <utility>
#include <algorithm>

struct ResourceBase {
    virtual ~ResourceBase() = default;
};

template<typename T>
struct Resource : public ResourceBase {
    std::shared_ptr<T> ptr;
    explicit Resource(std::shared_ptr<T> p) : ptr(std::move(p)) {}
    ~Resource() override = default;
};

class DeferredDeletionQueue {
private:
    std::vector<std::pair<std::unique_ptr<ResourceBase>, uint64_t>> resourcesToDelete;
    uint64_t currentFrame = 0;

    static DeferredDeletionQueue* instance;

    DeferredDeletionQueue() = default;

public:
    static DeferredDeletionQueue& get() {
        if (instance == nullptr) {
            instance = new DeferredDeletionQueue();
        }
        return *instance;
    }

    static void destroyInstance() {
        delete instance;
        instance = nullptr;
    }

    template<typename T>
    void pushResource(std::shared_ptr<T> resource) {
        auto wrapper = std::make_unique<Resource<T>>(std::move(resource));
        resourcesToDelete.push_back({ std::move(wrapper), currentFrame + 3 });
    }

    void update() {
        currentFrame++;
        resourcesToDelete.erase(
            std::remove_if(resourcesToDelete.begin(), resourcesToDelete.end(),
                [this](const auto& pair) { return pair.second <= currentFrame; }),
            resourcesToDelete.end());
    }
};