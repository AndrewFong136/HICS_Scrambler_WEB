const thbutton = document.querySelectorAll("th button");

let fetchedData = [];
let fetchedScrambledData = [];

function displayData(rows) {
    const teamsList = document.getElementById('teamsList');
    teamsList.innerHTML = ""; 

    const memberMap = {};

    rows.forEach(row => {
        const key = row.Members;

        if (!memberMap[key]) {
            memberMap[key] = {
                ...row,
                past_match: row.past_match ? [row.past_match] : []
            };
        } else {
            if (row.past_match) {
                memberMap[key].past_match.push(row.past_match);
            }
        }
    });

    Object.values(memberMap).forEach(row => {
        const tr = document.createElement('tr');

        const columns = [
            row.Members,
            row.Situation_Analyst,
            row.Solutions,
            row.Kabyas_Lapdog,
            row.Financials, 
            row.past_match.join(', ')
        ];
        
        columns.forEach(cell => {
            const td = document.createElement('td');
            td.textContent = cell || "-";
            tr.appendChild(td);
        });
        teamsList.appendChild(tr);
    });
}

function displayScrambledData(rows) {
    const scrambledTeams = document.getElementById("scrambledTeams");
    scrambledTeams.innerHTML = "";

    rows.forEach(row => {
        const tr = document.createElement("tr");

        const columns = [
            row.Team,
            row.Members,
            row.Roles
        ];

        columns.forEach(cell => {
            const td = document.createElement("td");
            td.textContent = cell || "-";
            tr.appendChild(td);
        });
        scrambledTeams.appendChild(tr);
    });

}

function fetchData() {
    fetch('http://localhost:5000/api/data')
        .then(response => response.json())
        .then(data => {
            fetchedData = data;
            displayData(data);
        })
}

function refreshData() {
    fetch('http://localhost:5000/api/refresh')
        .then(fetchData())
}

function scrambleData() {
    fetch('http://localhost:5000/api/scramble')
        .then(response => response.json())
        .then(data => {
            fetchedScrambledData = data;
            displayScrambledData(data);
        })
}

function updateHistory() {
    fetch('http://localhost:5000/api/update_history')
        .then(fetchData())

}

document.addEventListener('DOMContentLoaded', () => {
    refreshData();
    
    [...thbutton].map((button) => {

        button.addEventListener("click", (e) => {

            const tableId = e.target.closest("table").id;
           
            const direction = e.target.getAttribute("direction") || "asc";
            const newDirection = direction == "asc" ? "desc" : "asc";

            if (tableId == "teamsTable") {
                sortData(fetchedData, e.target.id, direction, displayData);
            } else if (tableId == "scrambledTeamsTable") {
                sortData(fetchedScrambledData, e.target.id, newDirection, displayScrambledData);
            }

            e.target.setAttribute("direction", newDirection);
        });

        button.addEventListener("mouseenter", () => {
            const img = document.createElement("img");
            img.src = "ui/updown_arrow.png";
            img.style.width = "10px";
            button.appendChild(img);
        })

        button.addEventListener("mouseleave", () => {
            const img = document.querySelector("img");
            button.removeChild(img);
        })
    });
});

setInterval(refreshData, 300000)