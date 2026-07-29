FROM emscripten/emsdk:6.0.5

RUN apt-get update && \
    apt-get install -y --no-install-recommends make cmake git && \
    apt-get autoremove -y && apt-get clean

RUN useradd -m mgba
VOLUME /home/mgba/src
USER mgba
WORKDIR /home/mgba/src
ENV BUILD_DIR=build-wasm

RUN git config --global --add safe.directory /home/mgba/src

CMD ["sh", "-c", "mkdir -p \"$BUILD_DIR\" && cd \"$BUILD_DIR\" && emcmake cmake .. $CMAKE_FLAGS && make install DESTDIR=install"]