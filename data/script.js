function openMode(evt, mode_name)
{
    var tab_content, tab_links, intro_text;

    // Grabs all elems. with tab_content class and hides them
    tab_links = document.getElementsByClassName("tablink");
    tab_content = document.getElementsByClassName("tabcontent");
    intro_text = document.getElementById("intro_text");

    if(intro_text.style.display != "none")
    {
        intro_text.style.display = "none";
    }

    for(let i = 0; i < tab_content.length; i++)
    {
        tab_content[i].style.display = "none";
    }

    document.getElementById(mode_name).style.display = "flex";
}

function modify_color(evt, strip_name)
{
    alert('Hellow');
}