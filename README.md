# Stock Market Simulator 📈💰

## Project Description
The **Stock Market Simulator** is a lightweight and interactive application designed for simulating real-time stock trading. With this simulator, users can trade stocks using virtual cash, track their portfolio performance, view transaction history, and compete with others on a global leaderboard. The application integrates with the **Finnhub API** to fetch real-time stock prices and news updates, while using **SQLite** for data storage to ensure smooth and efficient operations.

Whether you're learning about stock trading or just want to have some fun, this simulator offers a simple yet powerful experience!

## Technologies Used ⚙️
- **Programming Language:** C
- **Database:** SQLite
- **Networking Library:** libcurl
- **JSON Parsing Library:** cJSON
- **API Integration:** Finnhub API

## Features ✨
### Essential Features
- **User Authentication:** Secure login and signup with hashed passwords for enhanced security 🔒.
- **Portfolio Management:** Track owned stocks with details like quantity, purchase price, and current value 📊.
- **Transaction History:** Keep a log of all trades (buy/sell) with timestamps 🕒.
- **Profit and Loss Calculation:** View your net worth, combining cash balance and portfolio value 💵.
- **Command-line Interface (CLI):** Simple and user-friendly CLI for easy operations 🖥️.

### Advanced Features
- **Real-time Stock Prices:** Fetch live stock prices using the Finnhub API 📈.
- **News Updates:** Stay up-to-date with the latest stock market news 📰.
- **Leaderboard:** Compare your performance with other users based on net worth 🏆.
- **Graphical Data Representation:** Visualize trends and portfolio distribution with charts 📉.
- **Virtual Cash:** Start with a predefined amount (e.g., $10,000) for a simulated trading experience 💸.

## Database Design 🗄️
### Users Table
| Column         | Type    | Description                     |
|----------------|---------|---------------------------------|
| id             | INTEGER | Unique user ID.                 |
| username       | TEXT    | User's name.                    |
| password       | TEXT    | Hashed password.                |
| cash_balance   | REAL    | Virtual cash balance.           |

### Portfolio Table
| Column         | Type    | Description                     |
|----------------|---------|---------------------------------|
| id             | INTEGER | Unique entry ID.                |
| user_id        | INTEGER | Reference to the user.          |
| stock_symbol   | TEXT    | Stock ticker symbol.            |
| quantity       | INTEGER | Number of shares owned.         |
| purchase_price | REAL    | Price per share when bought.    |

### Transactions Table
| Column         | Type    | Description                     |
|----------------|---------|---------------------------------|
| id             | INTEGER | Unique transaction ID.          |
| user_id        | INTEGER | Reference to the user.          |
| stock_symbol   | TEXT    | Stock ticker symbol.            |
| transaction_type | TEXT  | "buy" or "sell".                |
| quantity       | INTEGER | Number of shares.               |
| price          | REAL    | Price per share at transaction. |
| timestamp      | TEXT    | Date and time of transaction.   |

### Leaderboard Table
| Column         | Type    | Description                     |
|----------------|---------|---------------------------------|
| user_id        | INTEGER | Reference to the user.          |
| net_worth      | REAL    | Cash balance + portfolio value. |
