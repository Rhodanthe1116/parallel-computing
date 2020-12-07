FROM gitpod/workspace-full

USER root

RUN apt-get update \
    && apt-get install -y libopenmpi-dev openmpi-bin openmpi-doc

ENV OMPI_MCA_btl_base_warn_component_unused 0

# Install custom tools, runtimes, etc.
# For example "bastet", a command-line tetris clone:
# RUN brew install bastet
#
# More information: https://www.gitpod.io/docs/config-docker/
