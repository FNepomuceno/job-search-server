let sort_column = '';
let sort_direction = 'ASC';

async function refresh_jobs() {
    let context = {};
    if (sort_column != '') {
        context['order-by'] = sort_column;
        context['order-direction'] = sort_direction;
    }
    let rows = JSON.parse(await http_get('/api/jobs', context)).result;

    rows.forEach((row) => {
        // Change "app link" to link
        let app_link = document.createElement('a');
        app_link.href = row['app_link'];
        app_link.innerHTML = 'Link';
        row['app_link'] = app_link;

        // Change interview details to edit
        let edit_link = document.createElement('a');
        let row_id = row['date_applied'].split(/[-: ]/).join('-');
        edit_link.href = `/jobs/${row_id}`;
        edit_link.innerHTML = 'Edit';
        row['interview_details'] = edit_link;
    });

    // Clear out table
    let table = document.getElementById("jobs_table");
    while (table.rows.length > 1) { table.deleteRow(1); }

    // Add result rows
    if (rows.length) {
        for (let row of rows) {
            let new_row = table.insertRow();
            for (let value of Object.values(row)) {
                let new_cell = new_row.insertCell();
                if (typeof value == "string") {
                    value = document.createTextNode(value);
                }
                new_cell.appendChild(value);
            }
        }
    } else {
        let new_row = table.insertRow();
        let new_cell = new_row.insertCell();
        new_cell.innerHTML = "No applications found";
        new_cell.colSpan = 12;
    }

    // Update "last updated" text
    let update_text = document.getElementById("last_updated");
    let options = {
        year: 'numeric',
        month: 'long',
        day: 'numeric',
        hour: 'numeric',
        minute: '2-digit'
    };
    let date_string = (new Date()).toLocaleDateString("en-US", options);
    update_text.innerHTML = `Last updated: ${date_string}`;
}

let table_updater = Updater(refresh_jobs, 60 * 1000);

function update_sorting(field, default_direction) {
    if (sort_column == field) {
        sort_direction = (sort_direction == 'ASC')? 'DESC': 'ASC';
    } else {
        sort_direction = default_direction;
    }
    sort_column = field;
    table_updater.update();
}

let company_header = document.getElementById('company_header');
company_header.onclick = update_sorting.bind(null, 'company', 'ASC');

let position_header = document.getElementById('position_header');
position_header.onclick = update_sorting.bind(null, 'position', 'ASC');

let location_header = document.getElementById('location_header');
location_header.onclick = update_sorting.bind(null, 'location', 'ASC');

let applied_header = document.getElementById('applied_header');
applied_header.onclick = update_sorting.bind(null, 'date_applied', 'DESC');

let updated_header = document.getElementById('updated_header');
updated_header.onclick =
    update_sorting.bind(null, 'latest_update', 'DESC');

window.onload = table_updater.update;
