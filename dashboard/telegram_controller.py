import asyncio
from telegram import Bot
from telegram import InputFile

async def send_file_via_telegram():
    # Replace 'YOUR_API_TOKEN' with your actual API token
    bot = Bot(token='5998765068:AAEfD2mbLqZTyqKaWU-rTQ5IXnOA6nczpzI')

    # Replace 'path_to_your_file' with the actual file path you want to send
    file_path = "logs/test.txt"

    # Replace 'your_chat_id' with the chat ID where you want to send the file
    # If you don't know the chat ID, you can retrieve it using the bot.get_updates() method
    chat_id = '781054200'

    # Send the file
    with open(file_path, 'rb') as file:
        # Send the file
        await bot.send_document(chat_id=chat_id, document=InputFile(file))

# Create an event loop to run the asynchronous function
if __name__ == "__main__":
    loop = asyncio.get_event_loop()
    loop.run_until_complete(send_file_via_telegram())