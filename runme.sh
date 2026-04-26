#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

make clean
make

touch testfile
rm -f stat.txt

pids=()

# Написать в runme.sh тестовый скрипт, который запустит в параллель 10 задач, каждая из которых в бесконечном цикле блокирует общий файл на 1 секунду, а потом снимает блокировку.
for i in {1..10}; do
    ./locker testfile &
    pids+=($!)
done

# Через пять минут завершить все десять задач сигналом sigint.
sleep 300

for p in "${pids[@]}"; do
    kill -INT "$p"
done

failed=0
for p in "${pids[@]}"; do
    wait "$p" || failed=$((failed + 1))
done

{
    echo "Test 1: stat file exists $( [ -f stat.txt ] && echo OK || echo FAIL )"
    echo

    # Убедиться, что файл статистики содержит по одной строке на каждую задачу.
    lines=$(wc -l < stat.txt)
    echo "Test 2: 10 lines in stat.txt: $lines $( [ "$lines" -eq 10 ] && echo OK || echo FAIL )"
    echo

    # Убедиться, что задачи не падают из-за порчи lck файла (гонки).
    echo "Test 3: no lock corruption (all exit 0): $( [ "$failed" -eq 0 ] && echo OK || echo "FAIL ($failed crashed)" )"
} > result.txt

echo "Done"