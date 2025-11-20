## Архитектура компилятора

### 1. Обзор компонентов

Компилятор состоит из **6 основных этапов**:

```
┌──────────────┐
│  Source Code │
└──────┬───────┘
       │
       ▼
┌─────────────────────────┐
│  1. Lexical Analysis    │  (lexer.h)
│  Tokens ◄─ Source Code  │
└──────┬──────────────────┘
       │
       ▼
┌─────────────────────────┐
│  2. Syntax Analysis     │  (parser.h)
│  AST ◄─ Tokens          │
└──────┬──────────────────┘
       │
       ▼
┌─────────────────────────┐
│  3. Semantic Analysis   │  (semantic.h)
│  Check types, scopes    │
│  Validated AST ◄        │
└──────┬──────────────────┘
       │
       ▼
┌─────────────────────────┐
│  4. Code Generation     │  (codegen.h)
│  IR (TAC) ◄ AST         │
└──────┬──────────────────┘
       │
       ▼
┌─────────────────────────┐
│  5. Optimization        │  (optimizer.h)
│  Optimized IR ◄         │
│  - Const folding        │
│  - Dead code elim.      │
└──────┬──────────────────┘
       │
       ▼
┌─────────────────────────┐
│  6. Interpretation      │  (interpreter.h)
│  Execute IR             │
│  Output ◄               │
└─────────────────────────┘
```

### 2. Ключевые классы и структуры

#### Lexer (lexer.h)
- **Входы**: Исходный код (строка)
- **Выход**: Вектор токенов
- **Функции**: 
  - Распознавание ключевых слов
  - Обработка комментариев (// и /* */)
  - Поддержка операторов и литералов

```cpp
std::vector<Token> tokens = lexer.tokenize();
```

#### Parser (parser.h)
- **Входы**: Вектор токенов
- **Выход**: Abstract Syntax Tree (AST)
- **Грамматика**: Рекурсивный спуск (recursive descent)
- **Приоритет операторов**: Правильно обработан

```cpp
Parser parser(tokens);
auto ast = parser.parse();
```

#### SemanticAnalyzer (semantic.h)
- **Входы**: AST
- **Выход**: Проверяет типы, области видимости
- **Функции**:
  - Проверка типов (type checking)
  - Таблица символов (symbol table)
  - Обнаружение ошибок (undefined variables, type mismatches)

```cpp
SemanticAnalyzer analyzer;
bool ok = analyzer.analyze(ast);
```

#### CodeGenerator (codegen.h)
- **Входы**: Валидированный AST
- **Выход**: Трехадресный код (Three-Address Code, TAC)
- **Функции**:
  - Преобразование выражений в промежуточный код
  - Управление метками (labels) для циклов и условных операторов
  - Распределение временных переменных (temp variables)

```cpp
CodeGenerator codegen(analyzer);
IRProgram ir = codegen.generate(ast);
```

#### Optimizer (optimizer.h)
- **Входы**: IR программа
- **Выход**: Оптимизированная IR
- **Оптимизации**:
  1. **Constant Folding** - вычисление констант на этапе компиляции
  2. **Dead Code Elimination** - удаление неиспользуемых инструкций

```cpp
Optimizer optimizer;
IRProgram optimized = optimizer.optimize(ir);
```

#### Interpreter (interpreter.h)
- **Входы**: IR программа
- **Выход**: Результаты исполнения
- **Функции**:
  - Виртуальная машина для TAC
  - Таблица переменных (variable store)
  - Обработка метток и переходов

```cpp
Interpreter interp;
interp.execute(ir);
```

### 3. Промежуточное представление (IR)

Трехадресный код (TAC) - это линейная последовательность инструкций вида:

```
x = y op z        (бинарная операция)
x = op y          (унарная операция)
x = y             (присваивание)
label:            (метка)
goto label        (безусловный переход)
ifz x goto label  (условный переход)
print(x)          (печать)
call f(a, b)      (вызов функции)
return x          (возврат)
```

### 4. Таблица символов

Для каждой переменной хранится:
- **Имя** (name)
- **Тип** (type: int, bool)
- **Строка определения** (line where defined)
- **Инициализирована ли** (initialized flag)

### 5. Типы инструкций

| Тип | Пример | Описание |
|-----|--------|---------|
| BIN_OP | `t1 = a + b` | Бинарная операция |
| UN_OP | `t1 = -a` | Унарная операция |
| ASSIGN | `x = y` | Присваивание |
| LABEL | `L1:` | Метка |
| GOTO | `goto L1` | Переход |
| IF_GOTO | `ifz t1 goto L1` | Условный переход |
| PRINT | `print(x)` | Вывод |
| CALL | `t1 = func(a, b)` | Вызов функции |
| RETURN | `return x` | Возврат |

### 6. Пример преобразований

**Исходный код:**
```java
int x = 5;
int y = 10;
int z = x + y * 2;
print(z);
```

**TAC (без оптимизации):**
```
1:  t1 = y * 2      // временная переменная t1
2:  t2 = x + t1
3:  z = t2
4:  print(z)
```

**TAC (с оптимизацией - constant folding):**
```
1:  t1 = 20         // 10 * 2 вычисляется как константа
2:  t2 = x + t1
3:  z = t2
4:  print(z)
```

### 7. Типы ошибок и предупреждений

#### Ошибки (приводят к отказу компиляции)
- Undefined variable: использование переменной, которая не была объявлена
- Variable already defined: повторное объявление переменной

#### Предупреждения (не приводят к отказу)
- Type mismatch: несовместимость типов в операции
- Uninitialized variable: использование неинициализированной переменной

### 8. Фазы выполнения

При запуске компилятор выполняет все 6 фаз последовательно, выводя статус каждой:

```
► Phase 1: Lexical Analysis
  ✓ Tokenization complete
  Tokens generated: 25

► Phase 2: Syntax Analysis
  ✓ Parsing complete
  Statements: 4

► Phase 3: Semantic Analysis
  ✓ Semantic analysis complete

► Phase 4: Code Generation
  ✓ Code generation complete
  Instructions: 8
  Variables: 3

► Phase 5: Optimization
  Dead code elimination: 2 instructions removed
  ✓ Optimization complete

► Phase 6: Interpretation
  ✓ Execution complete
  Output lines: 1

=== THREE-ADDRESS CODE (TAC) ===
0:  t1 = y * 2
1:  t2 = x + t1
...
```

---

**Автор**: Лабораторная работа 3  
**Курс**: Теория языков программирования и методы трансляции  
**Язык реализации**: C++17
