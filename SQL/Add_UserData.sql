USE [SSU_Project]
GO
/****** Object:  StoredProcedure [dbo].[Add_UserData]    Script Date: 2022-07-27 오후 6:11:34 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
-- =============================================
-- Author:		<Author,,Name>
-- Create date: <Create Date,,>
-- Description:	<Description,,>
-- =============================================
ALTER PROCEDURE [dbo].[Add_UserData] @Param0 NCHAR(10), @Param1 NCHAR(10), @Param2 NCHAR(10), @Param3 INT,@Param4 INT,@Param5 INT,@Param6 INT,
@Param7 INT,@Param8 INT,@Param9 INT,@Param10 INT,@Param11 INT,@Param12 INT,@Param13 INT
	-- Add the parameters for the stored procedure here
	
AS
IF NOT EXISTS (SELECT player_id FROM SSU_Table WHERE player_id = @Param0)
BEGIN
	-- SET NOCOUNT ON added to prevent extra result sets from
	-- interfering with SELECT statements.
	SET NOCOUNT ON;
    -- Insert statements for procedure here
	INSERT INTO SSU_Table(player_id,player_password,player_name, player_x,player_y,player_z, 
	player_hp, player_level, player_exp, player_maxhp, player_job, player_mp,player_maxmp, player_element)
	VALUES(@Param0,@Param1,@Param2,@Param3,@Param4,@Param5,@Param6,@Param7,@Param8,@Param9,@Param10,@Param11,@Param12,@Param13); 
END
ELSE  --존재하기 때문에 일부러 오류를 반환 
BEGIN
	INSERT INTO SSU_Table(player_id,player_password,player_name, player_x,player_y,player_z, 
	player_hp, player_level, player_exp, player_maxhp, player_job, player_mp,player_maxmp, player_element)
	VALUES(@Param3,@Param13,@Param2,@Param0,@Param4,@Param5,@Param6,@Param7,@Param8,@Param9,@Param10,@Param11,@Param12,@Param13); 
END

