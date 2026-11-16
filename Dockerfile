# Use Fedora 41 as base image
FROM fedora:41

# Switch to root to install packages
USER root

# Install required development tools and libraries


RUN dnf -y update && \
    dnf install -y \
      cmake gcc wget g++ git python3 redhat-lsb gcc-gfortran pcre-devel \
      mesa-libGL-devel mesa-libGLU-devel glew-devel ftgl-devel mariadb-devel fftw-devel \
      cfitsio-devel graphviz-devel libuuid-devel avahi-compat-libdns_sd-devel \
      openldap-devel python3-numpy libxml2-devel gsl-devel readline-devel \
      qt5-qtbase-devel qt5-qtwebengine-devel R-devel R-Rcpp-devel R-RInside-devel \
      libXpm-devel libXext-devel libXft-devel libX11-devel openssl-devel krb5-devel \
      xerces-c-devel tbb tbb-devel nodejs npm davix-devel clang clang-devel && \
    dnf clean all

# Create a non-root user (UID 8877) and set up home directory
RUN useradd -m -u 8877 seluser

# Switch to the non-root user
USER seluser
WORKDIR /home/seluser

RUN wget https://root.cern/download/root_v6.32.10.source.tar.gz && \
    mkdir root root_install && \
    tar -xf root_v6.32.10.source.tar.gz -C root --strip-components=1 && \
    rm root_v6.32.10.source.tar.gz && \
    mkdir root_build_temp && cd root_build_temp && \
    export CC=clang && export CXX=clang++ && \
    cmake \
        -DCMAKE_INSTALL_PREFIX=../root_install \
        -Dgnu_no_werror=ON \
        -DBUILD_TESTING=OFF \
        ../root && \
    make -j8 install && \
    cd .. && rm -rf root_build_temp root && \
    echo "source /home/seluser/root_install/bin/thisroot.sh" >> /home/seluser/.bashrc

WORKDIR /home/seluser
RUN source /home/seluser/root_install/bin/thisroot.sh && \
    git clone https://baltig.infn.it/gamma/agataselector.git && \
    mkdir agataselector_install && cd agataselector_install && \
    cmake \
        -DBUILD_SCRIPTS=ON \
        ../agataselector && \
    make -j8 && \
    echo "source /home/seluser/agataselector_install/Scripts/selector.sh" >> /home/seluser/.bashrc
    
### Install AGATA collaboration software via gRaySoftware ###
# Create a directory for gammaSoftware, clone the scripts repository, and run the installation commands.
RUN mkdir -p /home/seluser/gammaSoftware && \
    cd /home/seluser/gammaSoftware && \
    git clone https://gitlab.in2p3.fr/IPNL_GAMMA/scripts.git && \
    # Install ADF packages
    python3 scripts/gRaySoftware.py --adf= load conf && \
    python3 scripts/gRaySoftware.py --cmake=" -- -j8" --adf= compile install && \
    # Install Prisma packages
    #python3 gRaySoftware.py --prisma= load conf && \
    #python3 gRaySoftware.py --cmake=" -- -j8" --prisma= compile install && \
    # Install AGAPRO (including FEMUL)
    python3 scripts/gRaySoftware.py --agapro= load conf && \
    python3 scripts/gRaySoftware.py --cmake=" -- -j8" --agapro= compile install

# Add commands to source the gammaSoftware environment to .bashrc.
# Here we assume that the scripts/add2path.sh script should be sourced from /home/seluser/gammaSoftware.
RUN mkdir /home/seluser/host
RUN echo "cd /home/seluser/gammaSoftware && source scripts/add2path.sh /home/seluser/gammaSoftware /lib /bin >> /dev/null && export ADF_CONF_PATH=/home/seluser/gammaSoftware/agapro/femul && cd /home/seluser/host" >> /home/seluser/.bashrc
# Start bash with the customized .bashrc (which sources ROOT, GEANT4, and gammaSoftware environments)


#WORKDIR /host
CMD [ "/bin/bash", "-rcfile", "/home/seluser/.bashrc" ]
ENTRYPOINT ["/bin/bash", "-l", "-c"]
