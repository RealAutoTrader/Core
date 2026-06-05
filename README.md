# 자동매매 전략 엔진

## 소개

이 프로젝트는 실시간 시장 데이터를 기반으로 여러 매매 전략을 실행하고, 각 전략의 점수를 가중평균하여 최종 매매 신호를 생성하는 자동매매 전략 엔진입니다.

체결강도, 호가 불균형, 틱 이동평균, Z-score 전략을 활용하여 시장 상황을 분석하고, 최종적으로 BUY, SELL, HOLD 중 하나의 신호를 판단합니다.

## 주요 기능

- 실시간 종목 데이터 처리
- 체결강도 기반 매매 신호 생성
- 호가 불균형 기반 매매 신호 생성
- 틱 이동평균 기반 추세 판단
- Z-score 기반 평균회귀 전략 적용
- 4가지 전략 점수의 가중평균 계산
- BUY / SELL / HOLD 최종 신호 생성
- 리스크 관리 기능을 통한 매매 조건 검증

## 사용 방법

### 1. 프로젝트 다운로드

```bash
git clone [저장소 URL]
```

### 2. 프로젝트 폴더로 이동

```bash
cd [프로젝트 폴더명]
```

### 3. C++ 컴파일 환경 준비

C++17 이상을 지원하는 컴파일러를 준비합니다.

예시:

- g++
- clang++
- Visual Studio C++ Compiler

### 4. 소스 코드 컴파일

```bash
g++ -std=c++17 -o trading_bot \
main.cpp \
StrategyEngine.cpp \
RiskManager.cpp \
JsonUtils.cpp \
ExecutionStrengthStrategy.cpp \
OrderbookImbalanceStrategy.cpp \
TickMAStrategy.cpp \
ZScoreStrategy.cpp
```

### 5. 실행

```bash
./trading_bot
```

실행 후 입력되는 시장 데이터에 따라 전략 엔진이 매매 신호를 생성합니다.

## 프로젝트 구조

```text
.
├── main.cpp
├── StrategyEngine.cpp
├── StrategyEngine.hpp
├── StrategyConfig.hpp
├── SignalResult.hpp
├── SymbolState.hpp
├── RiskManager.cpp
├── RiskManager.hpp
├── JsonUtils.cpp
├── JsonUtils.hpp
├── ExecutionStrengthStrategy.cpp
├── ExecutionStrengthStrategy.hpp
├── OrderbookImbalanceStrategy.cpp
├── OrderbookImbalanceStrategy.hpp
├── TickMAStrategy.cpp
├── TickMAStrategy.hpp
├── ZScoreStrategy.cpp
└── ZScoreStrategy.hpp
```

## 전략 합산 방식

본 프로젝트는 여러 전략 중 하나를 우선순위로 선택하는 방식이 아니라, 각 전략이 생성한 점수를 가중평균하여 최종 매매 신호를 결정합니다.

각 전략은 시장 데이터를 분석하여 개별 score를 생성하고, StrategyEngine은 실행 가능한 전략들의 score에 weight를 곱해 합산합니다. 이후 실행된 전략들의 전체 weight로 나누어 최종 점수인 final_score를 계산합니다.

기본 전략 가중치는 다음과 같습니다.

| 전략 | 설명 | 가중치 |
|---|---|---:|
| Z-score 전략 | 평균회귀 기반 판단 | 0.25 |
| 체결강도 전략 | 매수/매도 체결 강도 분석 | 0.30 |
| 호가 불균형 전략 | 매수/매도 호가 잔량 불균형 분석 | 0.30 |
| 틱 이동평균 전략 | 단기/장기 틱 평균 기반 추세 판단 | 0.15 |

최종 점수 기준은 다음과 같습니다.

| final_score 범위 | 최종 신호 |
|---|---|
| 0.5 이상 | BUY |
| -0.5 이하 | SELL |
| 그 외 | HOLD |

따라서 최종 매매 신호는 단일 전략의 결과가 아니라, 여러 전략의 판단을 종합한 결과입니다.

## 라이선스

MIT License

본 프로젝트는 MIT 라이선스를 따릅니다.  
소스코드의 사용, 수정, 배포가 가능하며, 사용으로 인해 발생하는 문제에 대해서는 개발자가 책임지지 않습니다.
