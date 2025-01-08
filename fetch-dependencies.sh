# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    fetch-dependencies.sh                              :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: hasyxd <aliaudet@student.42lehavre.fr      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/01/08 13:47:18 by hasyxd            #+#    #+#              #
#    Updated: 2025/01/08 13:49:07 by hasyxd           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

#!/usr/bin/bash
if [ -d "dependencies" ]; then
	echo "Dependencies already installed"
	exit 0
fi

echo "Installing dependencies"

mkdir dependencies && cd dependencies

# Install virtualenv locally if not available
if ! python3 -m pip show virtualenv > /dev/null 2>&1; then
    echo "Installing virtualenv locally..."
    python3 -m pip install --user virtualenv
fi

if python3 -c "help('modules')" | grep virtualenv; then
	python3 -m virtualenv venv
else
	python3 -m venv venv
fi

source ./venv/bin/activate

# Install CLI tools
pip install --upgrade pip --no-warn-script-location
pip install glad
pip install glfw

# glad
glad --profile="core" --api="gl=4.5" --generator="c" --spec="gl" --extensions="" --out-path="." --no-loader
mv include/* . && mv src/glad.c glad/glad.c && rm -rf include src

# glfw
curl -LOs https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip
unzip -q glfw-3.4.zip && rm glfw-3.4.zip

cmake glfw-3.4/ -B glfw-3.4/build/
cd glfw-3.4/build;make
cd ../../
mkdir glfw
mv glfw-3.4/build/src/libglfw3.a glfw/
mv glfw-3.4/include/GLFW/* glfw/
rm -rf glfw-3.4

deactivate
rm -rf venv
echo "Dependencies installed"
