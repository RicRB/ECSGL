struct Mount {
    std::filesystem::path root;
    int priority; // higher wins
    bool isPack;  // if you later add zip/pak
};

class VFS {
public:
    void mount(std::filesystem::path root, int priority) { mounts.push_back({ std::move(root), priority, false }); }
    std::filesystem::path resolve(const std::string& vpath) const {
        std::vector<Mount> m = mounts;
        std::sort(m.begin(), m.end(), [](auto& a, auto& b) { return a.priority > b.priority; });
        for (auto& mt : m) {
            auto p = mt.root / vpath;
            if (std::filesystem::exists(p)) return std::filesystem::canonical(p);
        }
        return {};
    }
private:
    std::vector<Mount> mounts;
};