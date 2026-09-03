extern "C" int puts(const char*);

int main() {
    puts(R"HTML(<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>大学迎新破冰：交友 Bingo 挑战赛</title>
    <script src="https://cdn.tailwindcss.com"></script>
    <style>
        body { font-family: system-ui, -apple-system, sans-serif; }
    </style>
</head>
<body class="bg-slate-50 min-h-screen py-8 px-4 text-slate-800">
    <div class="max-w-4xl mx-auto">
        <!-- 头部说明 -->
        <header class="text-center mb-6">
            <h1 class="text-3xl font-bold text-indigo-600 mb-2">🎓 大学迎新破冰：交友 Bingo</h1>
            <p class="text-sm text-slate-600 max-w-xl mx-auto">
                点击任意格子输入新朋友的签名并打勾！横、竖、斜任意连成 <span class="font-bold text-indigo-600">1 条直线</span> 即可触发 BINGO！
            </p>
            <div id="status-banner" class="mt-4 inline-block bg-indigo-50 border border-indigo-200 text-indigo-700 px-4 py-2 rounded-full text-sm font-semibold transition-all">
                当前进度：已完成 0 个格子 | 尚未连线
            </div>
        </header>

        <!-- Bingo 表格区域 -->
        <div class="bg-white shadow-xl rounded-2xl p-4 md:p-6 overflow-x-auto">
            <div class="grid grid-cols-5 gap-2 md:gap-3 min-w-[700px]" id="bingo-grid">
                <!-- 动态生成 25 个格子 -->
            </div>
        </div>

        <!-- 底部操作按钮 -->
        <div class="mt-6 flex justify-center gap-4">
            <button onclick="resetBoard()" class="bg-slate-200 hover:bg-slate-300 text-slate-700 font-medium px-5 py-2.5 rounded-xl transition-all shadow-sm">
                清空重置
            </button>
            <button onclick="checkBingo()" class="bg-indigo-600 hover:bg-indigo-700 text-white font-medium px-6 py-2.5 rounded-xl transition-all shadow-md">
                手动检查连线
            </button>
        </div>
    </div>

    <!-- 自定义签名填写弹窗 -->
    <div id="name-modal" class="fixed inset-0 bg-black/50 hidden flex items-center justify-center p-4 z-50">
        <div class="bg-white rounded-2xl p-6 max-w-sm w-full shadow-2xl">
            <h3 id="modal-title" class="text-lg font-bold text-slate-800 mb-1">填写签名</h3>
            <p class="text-xs text-slate-500 mb-4" id="modal-desc"></p>
            <input type="text" id="signer-name" placeholder="请输入同学的名字..." class="w-full border border-slate-300 rounded-xl px-4 py-2.5 mb-4 focus:outline-none focus:ring-2 focus:ring-indigo-500">
            <div class="flex justify-end gap-2">
                <button onclick="closeModal()" class="px-4 py-2 bg-slate-100 hover:bg-slate-200 text-slate-600 rounded-xl text-sm font-medium">取消</button>
                <button onclick="saveSigner()" class="px-4 py-2 bg-indigo-600 hover:bg-indigo-700 text-white rounded-xl text-sm font-medium">确认</button>
            </div>
        </div>
    </div>

    <!-- BINGO 成功弹窗 -->
    <div id="bingo-modal" class="fixed inset-0 bg-black/60 hidden flex items-center justify-center p-4 z-50">
        <div class="bg-white rounded-3xl p-8 max-w-md w-full shadow-2xl text-center">
            <div class="text-5xl mb-3">🎉</div>
            <h2 class="text-2xl font-black text-indigo-600 mb-2">BINGO 成功！</h2>
            <p class="text-slate-600 mb-6">太棒了！你已经成功连成一线，快去找主持人验证领奖吧！</p>
            <button onclick="closeBingoModal()" class="bg-indigo-600 hover:bg-indigo-700 text-white font-semibold px-6 py-3 rounded-2xl w-full shadow-lg">
                继续游戏
            </button>
        </div>
    </div>

    <script>
        // 最新版本的 25 个无重复交友挑战条件
        const bingoTasks = [
            "名字是 A 开头的", "是独生子女", "家里养了猫或狗", "假期喜欢宅在家不出门", "夜猫子",
            "Perlis 本地人", "超级擅长随时随地睡觉", "至少去过 3 个不同的国家", "咖啡因重度依赖者", "喜欢看悬疑/科幻电影",
            "正在学习一门新语言", "会弹某种乐器", "能唱出 KMP 校歌的人", "拥有驾照但不敢上路", "宿舍里常备零食",
            "拥有一个冷门的小众收藏", "能背出圆周率小数点后 10 位", "喜欢早起的人", "擅长打游戏", "喜欢户外徒步或露营",
            "手机相册有 1W+ 张照片", "讨厌吃香菜", "拥有某种隐藏特长", "国庆月生日的人", "随身带着耳机走哪听哪"
        ];

        let boardState = Array(25).fill({ checked: false, name: '' });
        let activeIndex = null;

        const gridEl = document.getElementById('bingo-grid');
        const modalEl = document.getElementById('name-modal');
        const bingoModalEl = document.getElementById('bingo-modal');
        const signerInput = document.getElementById('signer-name');
        const modalTitle = document.getElementById('modal-title');
        const modalDesc = document.getElementById('modal-desc');
        const statusBanner = document.getElementById('status-banner');

        function renderBoard() {
            gridEl.innerHTML = '';
            let completedCount = 0;

            bingoTasks.forEach((task, index) => {
                const cellData = boardState[index];
                if (cellData.checked) completedCount++;

                const cell = document.createElement('div');
                cell.className = `p-3 rounded-xl border flex flex-col justify-between min-h-[110px] cursor-pointer transition-all select-none ${
                    cellData.checked 
                        ? 'bg-indigo-50 border-indigo-300 shadow-sm' 
                        : 'bg-white border-slate-200 hover:border-indigo-300 hover:shadow-md'
                }`;
                
                cell.innerHTML = `
                    <div>
                        <div class="text-[10px] font-bold text-slate-400 mb-1">#${index + 1}</div>
                        <p class="text-xs font-semibold text-slate-700 leading-snug">${task}</p>
                    </div>
                    <div class="mt-2 pt-2 border-t border-slate-100 flex items-center justify-between">
                        <span class="text-[11px] truncate max-w-[110px] ${cellData.name ? 'text-indigo-600 font-bold' : 'text-slate-400 italic'}">
                            ${cellData.name ? '签名: ' + cellData.name : '点击填写签名'}
                        </span>
                        <div class="w-5 h-5 rounded-full flex items-center justify-center text-xs ${cellData.checked ? 'bg-indigo-600 text-white' : 'border border-slate-300'}">
                            ${cellData.checked ? '✓' : ''}
                        </div>
                    </div>
                `;

                cell.onclick = () => openModal(index);
                gridEl.appendChild(cell);
            });

            const hasBingo = checkWinningLines(false);
            statusBanner.innerHTML = `当前进度：已完成 <b>${completedCount}</b> / 25 个格子 | ${hasBingo ? '<span class="text-emerald-600 font-bold">🎉 已达成 BINGO！</span>' : '尚未连线'}`;
        }

        function openModal(index) {
            activeIndex = index;
            modalTitle.innerText = `格子 #${index + 1}`;
            modalDesc.innerText = bingoTasks[index];
            signerInput.value = boardState[index].name || '';
            modalEl.classList.remove('hidden');
            signerInput.focus();
        }

        function closeModal() {
            modalEl.classList.add('hidden');
            activeIndex = null;
        }

        function saveSigner() {
            if (activeIndex === null) return;
            const name = signerInput.value.trim();
            boardState[activeIndex] = {
                checked: name.length > 0,
                name: name
            };
            closeModal();
            renderBoard();
        }

        function resetBoard() {
            boardState = Array(25).fill({ checked: false, name: '' });
            renderBoard();
        }

        function checkWinningLines(showAlert = true) {
            const matrix = [];
            for (let i = 0; i < 5; i++) {
                matrix.push(boardState.slice(i * 5, i * 5 + 5).map(c => c.checked));
            }

            let lines = 0;
            // 横向
            for (let i = 0; i < 5; i++) {
                if (matrix[i].every(val => val)) lines++;
            }
            // 纵向
            for (let j = 0; j < 5; j++) {
                if ([0,1,2,3,4].every(i => matrix[i][j])) lines++;
            }
            // 对角线
            if ([0,1,2,3,4].every(i => matrix[i][i])) lines++;
            if ([0,1,2,3,4].every(i => matrix[i][4 - i])) lines++;

            if (showAlert) {
                if (lines > 0) {
                    bingoModalEl.classList.remove('hidden');
                } else {
                    alert("目前还没有连线哦，加油继续找新朋友签名吧！");
                }
            }
            return lines > 0;
        }

        function checkBingo() {
            checkWinningLines(true);
        }

        function closeBingoModal() {
            bingoModalEl.classList.add('hidden');
        }

        // 初始化渲染
        renderBoard();
    </script>
</body>
</html>
)HTML");
    return 0;
}