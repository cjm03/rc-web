// NavBar
function hideIconBar(){
    var iconBar = document.getElementById("iconBar");
    var navigation = document.getElementById("navigation");
    iconBar.setAttribute("style", "display:none;");
    navigation.classList.remove("hide");
}

function showIconBar(){
    var iconBar = document.getElementById("iconBar");
    var navigation = document.getElementById("navigation");
    iconBar.setAttribute("style", "display:block;");
    navigation.classList.add("hide");
}

// Show Comment area
function showComment(){
    var commentArea = document.getElementById("comment-area");
    commentArea.classList.remove("hide");
}

// Show Reply area
function showReply(){
    var replyArea = document.getElementById("reply-area");
    replyArea.classList.remove("hide");
}

// --- Forum Logic ---

// Add a new comment (detail.html)
document.addEventListener('DOMContentLoaded', function() {
    // Comment submit
    const commentSubmit = document.querySelector('#comment-area input[type="submit"]');
    if(commentSubmit){
        commentSubmit.addEventListener('click', function(e){
            e.preventDefault();
            const textarea = document.querySelector('#comment-area textarea');
            const comments = getComments();
            const commentText = textarea.value.trim();
            if(commentText){
                comments.push({user: 'CurrentUser', text: commentText, replies: []});
                saveComments(comments);
                textarea.value = '';
                renderComments();
                document.getElementById("comment-area").classList.add("hide");
            }
        });
    }

    // Reply submit
    const replySubmit = document.querySelectorAll('#reply-area input[type="submit"]');
    replySubmit.forEach((btn, idx) => {
        btn.addEventListener('click', function(e){
            e.preventDefault();
            const textarea = btn.parentElement.querySelector('textarea');
            const replyText = textarea.value.trim();
            if(replyText){
                // For demo, always reply to the first comment
                const comments = getComments();
                if(comments.length > 0){
                    comments[0].replies.push({user: 'CurrentUser', text: replyText});
                    saveComments(comments);
                    renderComments();
                    textarea.value = '';
                    btn.parentElement.classList.add("hide");
                }
            }
        });
    });

    // Initial render (detail.html)
    if(document.querySelector('.comments-container')){
        renderComments();
    }

    // Search filter (posts.html)
    const searchInput = document.querySelector('.search-box input[type="text"]');
    if(searchInput){
        searchInput.addEventListener('input', function(){
            const q = searchInput.value.trim().toLowerCase();
            const posts = document.querySelectorAll('.table-row');
            posts.forEach(row => {
                const subject = row.querySelector('.subjects a').textContent.toLowerCase();
                const desc = row.querySelector('.subjects').textContent.toLowerCase();
                if(subject.includes(q) || desc.includes(q)){
                    row.style.display = '';
                } else {
                    row.style.display = 'none';
                }
            });
        });
    }
});

// --- Helper functions for comments (detail.html) ---
function getComments(){
    try {
        return JSON.parse(localStorage.getItem('forum_comments')) || [];
    } catch(e){
        return [];
    }
}
function saveComments(comments){
    localStorage.setItem('forum_comments', JSON.stringify(comments));
}
function renderComments(){
    const comments = getComments();
    const container = document.querySelectorAll('.comments-container');
    // Clear existing
    container.forEach((c, i) => {
        c.innerHTML = '';
    });

    comments.forEach((cmt, idx) => {
        const div = document.createElement('div');
        div.className = 'body';
        div.innerHTML = `
            <div class="authors">
                <div class="username"><a href="">${cmt.user}</a></div>
                <div>Role</div>
                <img src="https://cdn.pixabay.com/photo/2015/11/06/13/27/ninja-1027877_960_720.jpg" alt="">
                <div>Posts: <u>1</u></div>
                <div>Points: <u>10</u></div>
            </div>
            <div class="content">
                ${escapeHTML(cmt.text)}
                <br>
                <div class="comment">
                    <button onclick="showReply()">Reply</button>
                </div>
            </div>
        `;
        if(container[idx]) container[idx].appendChild(div);
        // Render replies
        if(cmt.replies && cmt.replies.length > 0){
            cmt.replies.forEach(rep => {
                const replyDiv = document.createElement('div');
                replyDiv.className = 'body reply';
                replyDiv.innerHTML = `
                    <div class="authors">
                        <div class="username"><a href="">${rep.user}</a></div>
                        <img src="https://cdn.pixabay.com/photo/2015/11/06/13/27/ninja-1027877_960_720.jpg" alt="">
                    </div>
                    <div class="content">
                        ${escapeHTML(rep.text)}
                    </div>
                `;
                container[idx].appendChild(replyDiv);
            });
        }
    });
}

// Utility
function escapeHTML(str){
    return str.replace(/[&<>"']/g, function(m) {
        return ({
            '&':'&amp;',
            '<':'&lt;',
            '>':'&gt;',
            '"':'&quot;',
            "'":'&#39;'
        })[m];
    });
}
