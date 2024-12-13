[11:01, 12/13/2024] GO: # Stock Market Simulator 📈💰

## Project Description
The *Stock Market Simulator* is a lightweight and interactive application designed for simulating real-time stock trading. With this simulator, users can trade stocks using virtual cash, track their portfolio performance, view transaction history, and compete with others on a global leaderboard. The application integrates with the *Finnhub API* to fetch real-time stock prices and news updates, while using *SQLite* for data storage to ensure smooth and efficient operations.

Whether you're learning about stock trading or just want to have some fun, this simulator offers a simple yet powerful experience!

## Technologies Used ⚙️
- *Programming Language:* C
- *Database:* SQLite
- *Networking Library:* libcurl
- …
[11:02, 12/13/2024] GO: markdown
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

## Installation Instructions 🛠️

### Step 1: Set Up Your Environment
1. **Install a C Compiler:**
   - **Linux/macOS:** `gcc`
   - **Windows:** MinGW or an IDE like Code::Blocks.

2. **Install SQLite:**
   - **Linux:** `sudo apt install sqlite3 libsqlite3-dev`
   - **Windows:** Download precompiled binaries from the [SQLite Downloads page](https://sqlite.org/download.html).

3. **Install libcurl:**
   - **Linux:** `sudo apt install libcurl4-openssl-dev`
   - **Windows:** Download and include libcurl headers and libraries.

4. **Install cJSON:**
   - Download `cJSON.c` and `cJSON.h` from the [cJSON GitHub repository](https://github.com/DaveGamble/cJSON).

### Step 2: Set Up the Project
1. Create a project directory with the following structure:
   plaintext
   stock_simulator/
   ├── main.c
   ├── database.c
   ├── database.h
   ├── api.c
   ├── api.h
   ├── auth.c
   ├── auth.h
   ├── sqlite3.c
   ├── cJSON.c
   ├── Makefile
   
2. Place the SQLite source files, cJSON files, and your code files in the appropriate locations.

3. Compile the program using:
   bash
   gcc -o stock_simulator main.c database.c api.c auth.c sqlite3.c cJSON.c -lcurl -lsqlite3
   

## Usage Instructions 🚀
1. Run the program:
   bash
   ./stock_simulator
   

2. Use the CLI to:
   - Sign up or log in 📝.
   - View live stock prices 💹.
   - Buy or sell stocks 📈📉.
   - View your transaction history 🧾.
   - Check your portfolio 📊.
   - Compete on the leaderboard 🏅.

## Roadmap 🚧
- Add user-friendly CLI menus for better navigation 🎮.
- Implement graphical data visualization 📉📊.
- Add multi-user support with a centralized server 🌐.

## Contribution Guidelines 🤝
1. **Fork the repository.**
2. **Create a feature branch.**
3. **Commit your changes** with descriptive messages 📝.
4. **Open a pull request** for review 🔄.

## Contact Information 📬
For any queries or suggestions, feel free to reach out:
- **Email:** [your-email@example.com]
- **GitHub:** [your-github-profile]

## License Information 📜
This project is licensed under the MIT License. See the LICENSE file for more details.

---

Happy trading and coding! 🚀📈💸
