
#pragma once

#include <unistd.h>
#include <utility>

/// Provides a std::unique_ptr like wrapper for a file descriptor like a socket handle
class unique_fd {
public:
    constexpr unique_fd() noexcept = default;

    explicit unique_fd(int fd) noexcept : fd_(fd) { }

    unique_fd(unique_fd &&u) noexcept : fd_(u.fd_) { u.fd_ = -1; }

    /// Close the file descriptor on destruction
    ~unique_fd()
    {
        if (-1 != fd_)
        {
            close();
        }
    }

    unique_fd &operator=(unique_fd &&u) noexcept {
        reset(u.release());
        return *this;
    }

    int get() const noexcept { return fd_; }

    operator int() const noexcept { return fd_; }

    /// Release the stored filedescriptor from this container and dont guard it anymore in it
    int release() noexcept {
        int fd = fd_;
        fd_ = -1;
        return fd;
    }

    void reset(int fd = -1) noexcept { unique_fd(fd).swap(*this); }

    void swap(unique_fd &u) noexcept { std::swap(fd_, u.fd_); }

    unique_fd(const unique_fd &) = delete;

    unique_fd &operator=(const unique_fd &) = delete;

    /// Close the filedescriptor
    /// \return Returns 0 if close was successful. Otherwise check errno
    int close() noexcept {
        if (-1 == fd_)
            return 0;

        int closed = ::close(fd_);
        fd_ = -1;
        return closed;
    }

    /// Implicit check if container has a filedescriptor
    /// \return True if it has a filedescriptor
    operator bool() const noexcept {
        return fd_ >= 0;
    }

    /// Compareoperation for sorting
    /// \param rhs Other filedescriptor to compare
    /// \return True if this is smaller than given parameter
    bool operator<(const int& rhs) const
    {
        return fd_ < rhs;
    }
private:
    int fd_ = -1;
};

