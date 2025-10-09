# Результаты лексического анализа

| Строка | Имя лексемы | Класс | Значение/Атрибут |
|--------|-------------|-------|------------------|
| 1 | `#` | ERROR | lexical_error |
| 1 | `include` | IDENTIFIER | id_1_2 |
| 1 | `<` | OPERATOR | - |
| 1 | `iostream` | IDENTIFIER | id_1_11 |
| 1 | `>` | OPERATOR | - |
| 2 | `using` | IDENTIFIER | id_2_1 |
| 2 | `namespace` | IDENTIFIER | id_2_7 |
| 2 | `std` | IDENTIFIER | id_2_17 |
| 2 | `;` | DELIMITER | - |
| 4 | `int` | KEYWORD | - |
| 4 | `main` | IDENTIFIER | id_4_5 |
| 4 | `(` | DELIMITER | - |
| 4 | `)` | DELIMITER | - |
| 4 | `{` | DELIMITER | - |
| 5 | `return` | KEYWORD | - |
| 5 | `0` | INTEGER | 0 |
| 5 | `;` | DELIMITER | - |
| 6 | `}` | DELIMITER | - |

## Статистика

| Тип токена | Количество |
|------------|------------|
| INTEGER | 1 |
| KEYWORD | 2 |
| OPERATOR | 2 |
| DELIMITER | 6 |
| IDENTIFIER | 6 |
| ERROR | 1 |

---
**Использованные методы:**
- Расширяемый конечный автомат (9 баллов)
- Хеширование для ключевых слов (5 баллов)
- Бинарный поиск для операторов (5 баллов)
- Обработка комментариев (+2 балла)

**Итого: 21 балл**
