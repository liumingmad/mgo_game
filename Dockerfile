# 使用Ubuntu 22.04作为基础镜像
FROM ubuntu:22.04 AS builder

# 避免交互式安装
ENV DEBIAN_FRONTEND=noninteractive

# 安装编译依赖 (只包含客户端库，不安装MySQL和Redis服务)
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libssl-dev \
    libcurl4-openssl-dev \
    libcrypto++-dev \
    libmysqlcppconn-dev \
    libhiredis-dev \
    libspdlog-dev \
    libfmt-dev \
    && rm -rf /var/lib/apt/lists/*

# 安装redis++库 (从源码编译，因为Ubuntu 22.04没有预编译包)
RUN git clone https://github.com/sewenew/redis-plus-plus.git /tmp/redis-plus-plus && \
    cd /tmp/redis-plus-plus && \
    mkdir build && cd build && \
    cmake -DREDIS_PLUS_PLUS_CXX_STANDARD=20 .. && \
    make -j$(nproc) && \
    make install && \
    ldconfig && \
    rm -rf /tmp/redis-plus-plus

# 安装jwt-cpp库 (header-only库)
RUN git clone https://github.com/Thalhammer/jwt-cpp.git /tmp/jwt-cpp && \
    cd /tmp/jwt-cpp && \
    mkdir build && cd build && \
    cmake -DJWT_BUILD_EXAMPLES=OFF .. && \
    make -j$(nproc) && \
    make install && \
    rm -rf /tmp/jwt-cpp

# 创建工作目录
WORKDIR /app

# 复制源代码
COPY . .

# 创建构建目录并编译 (禁用测试以避免依赖测试文件)
RUN mkdir -p build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_BOARD_TESTS=OFF .. && \
    make -j$(nproc)

# 运行时镜像
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# 安装运行时依赖 (只包含客户端库，不安装MySQL和Redis服务)
RUN apt-get update && apt-get install -y \
    libssl3 \
    libcurl4 \
    libcrypto++8 \
    libmysqlcppconn-dev \
    libhiredis0.14 \
    libspdlog1 \
    libfmt8 \
    && rm -rf /var/lib/apt/lists/*

# 复制redis++运行时库
COPY --from=builder /usr/local/lib/libredis++* /usr/local/lib/
RUN ldconfig

# 创建应用目录
WORKDIR /app

# 复制编译好的可执行文件
COPY --from=builder /app/build/main/Mgo /app/

# 复制配置文件
COPY --from=builder /app/config/ /app/config/

# 创建日志目录
RUN mkdir -p /app/logs

# 创建非root用户
RUN groupadd -r appuser && useradd -r -g appuser appuser && \
    chown -R appuser:appuser /app

USER appuser

# 暴露端口 (根据你的服务器配置调整)
EXPOSE 8080

# 启动应用
CMD ["./Mgo"]