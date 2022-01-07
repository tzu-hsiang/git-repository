import asyncio
from pyppeteer import launch
import random
import time

async def scraper():

    browser = await launch( headless = False, args=['--start-maximized'], ignoreDefaultArgs= ['--enable-automation'] )
    #browser = await launch( headless = True, ignoreDefaultArgs= ['--enable-automation'] )
    page = await browser.newPage()
    await page.setViewport({'width': 1920, 'height': 1080})
    await page.setUserAgent('Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/96.0.4664.110 Safari/537.36')
    await page.goto('https://mee6.xyz/leaderboard/919566117612187679')
    await page.evaluateOnNewDocument( '() =>{ Object.defineProperties(navigator,{ webdriver:{ get: () => false } }) }')

    print('前30名名單讀取/Fetch List')
    list_30 = []
    
    for row in range(1, 31):
        tb_row_id = '#app-mount > div > div > div.leaderboardBody > div.leaderboardPlayersListContainer > div > div > div:nth-child(' + str(row) + ') > div.leaderboardPlayer > div.leaderboardPlayerLeft > div.leaderboardPlayerUsername'
        element = await page.waitForSelector(tb_row_id, {'timeout': 1000 * 20})
        id = await(await element.getProperty('textContent')).jsonValue()
        list_30.append(id)
        print(id)
        
    # for
    await browser.close()
    
    print('=====================\n參加者名單讀取/Fetch List')
    list_pt = []
    
    f = open('pt.txt', 'r', encoding = 'utf-8')
    list_pt = f.read().splitlines()
    
    for i, elem in enumerate(list_pt):
        print("第{n}位參加者: id: {id}".format(n = i + 1, id = elem))
    
    print('=====================\n開始抽選\Lottery')
    choice = 'Y'
    num = 0
    
    while choice.upper() == 'Y' and num < 5  :
        rand = random.randint(0, len(list_30) - 1)
        print("第{n}位: id: {id}".format(n = num + 1, id = list_30[rand]))
        
        if list_30[rand] not in list_pt:
            print('未參加本次抽獎，reroll\n')
            continue
        
        choice = input('其他原因:reroll? (Y/N):')
        

        if choice.upper() == 'N':
            print('已記錄獲獎名單!')
            fw = open('winner.txt', 'a', encoding = 'utf-8')
            fw.write(list_30[rand])
            fw.write('\n')
            print('\n')
            list_30.remove(list_30[rand])
            num += 1
            choice = 'Y'
        else:
            print('\n')
    # while

    print('\n=====================\n以上每人獎勵2000 $YLTR，恭喜獲獎!')
    print('Congratulations to all of the above for winning 2000 $YLTR')
    
if __name__ == '__main__':
    print('現在時間/current time: ', end = '')
    struct_time = time.localtime(int(time.time()))
    time_str = time.strftime('%Y.%m.%d %H:%M:%S (UTC+8)', struct_time)
    print(time_str)
    asyncio.get_event_loop().run_until_complete(scraper())
