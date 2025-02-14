# Имя конечного исполняемого файла
TARGET = cpp-transport-catalogue

# Источники и объекты
SRC_DIR = ./transport-catalogue
BUILD_DIR = build
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_FILES))

# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++20

# Основная цель
all: $(BUILD_DIR)/$(TARGET)

# Сборка исполняемого файла
$(BUILD_DIR)/$(TARGET): $(OBJ_FILES)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(OBJ_FILES) -o $@

# Правило для создания объектного файла
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Цель для полной пересборки
rebuild: clean all

# Очистка
clean:
	rm -rf $(BUILD_DIR)/$(TARGET) $(BUILD_DIR)/*.o

.PHONY: all rebuild clean
